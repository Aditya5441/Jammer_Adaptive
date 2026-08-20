#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN_WIFI 22
#define CSN_PIN_WIFI 21
#define CE_PIN_BT 25
#define CSN_PIN_BT 15

#define BLOCK_SIZE 16
#define NUM_ROUNDS_DEFAULT 32
#define HOP_LEN 16
#define NOISE_HISTORY_LEN 64

const unsigned long channelDelayBase = 10;

// WiFi channels 1-11 mapped to nRF24L01 channel numbers
const uint8_t wifiChannels[11] = {
  12, // WiFi ch1 (2412 MHz)
  22, // WiFi ch2 (2422 MHz)
  32, // WiFi ch3 (2432 MHz)
  42, // WiFi ch4 (2442 MHz)
  52, // WiFi ch5 (2452 MHz)
  62, // WiFi ch6 (2462 MHz)
  72, // WiFi ch7 (2472 MHz)
  18,
  26,
  38,
  48
};

// Bluetooth 2.402-2.480 GHz mapped to nRF24L01 channels 2-80
const uint8_t btChannels[79] = {
  2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,
  22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
  41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,
  61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80
};

RF24 wifiJammer(CE_PIN_WIFI, CSN_PIN_WIFI);
RF24 btJammer(CE_PIN_BT, CSN_PIN_BT);

enum Mode { WIFI_JAMMING, BT_JAMMING };
Mode currentMode = WIFI_JAMMING;
unsigned long modeStartMillis = 0;
const unsigned long modeDuration = 60000;

unsigned int noise_buffer[NOISE_HISTORY_LEN];
int noise_idx = 0;

unsigned char interferencePayload[32] = "NOISE_INTERFERENCE_DATA_123456";

typedef enum { XOR_OP, ADD_OP, ROTL_OP } OpType;
typedef struct {
  OpType ops[3];
} MorphParams;

const double LOGISTIC_R = 3.999;
double logistic_x = 0.501;

unsigned int read_radio_noise() {
  static unsigned int val = 0xACE1U;
  val ^= val << 13; val ^= val >> 17; val ^= val << 5;
  return val;
}

unsigned int fused_entropy_source() {
  static unsigned long last_time = 0;
  unsigned long current_time = micros();
  unsigned int noise = read_radio_noise();
  unsigned int jitter = (unsigned int)(current_time - last_time);
  last_time = current_time;
  unsigned int entropy = noise ^ jitter;
  entropy ^= (entropy << 7) ^ (entropy >> 11) ^ (entropy << 13);
  return entropy;
}

void update_noise_history() {
  noise_buffer[noise_idx] = read_radio_noise();
  noise_idx = (noise_idx + 1) % NOISE_HISTORY_LEN;
}

unsigned char tda_feature_extraction() {
  unsigned int count = 0;
  for (int i = 1; i < NOISE_HISTORY_LEN; i++) {
    if (((noise_buffer[i] ^ noise_buffer[i - 1]) & 0x1) != 0) count++;
  }
  return (unsigned char)(count & 0xFF);
}

int dynamic_key_length(unsigned int entropy) {
  return 12 + (entropy % 9);
}

void adaptive_morph_params(MorphParams *params, unsigned char tda_val) {
  if (tda_val > 40) {
    params->ops[0] = ROTL_OP; params->ops[1] = ADD_OP; params->ops[2] = XOR_OP;
  } else if (tda_val > 20) {
    params->ops[0] = XOR_OP; params->ops[1] = ROTL_OP; params->ops[2] = ADD_OP;
  } else {
    params->ops[0] = ADD_OP; params->ops[1] = XOR_OP; params->ops[2] = ROTL_OP;
  }
}

unsigned char op_xor(unsigned char a, unsigned char b) { return a ^ b; }
unsigned char op_add(unsigned char a, unsigned char b) { return (unsigned char)((a + b) & 0xFF); }
unsigned char op_rotl(unsigned char a, unsigned char b) {
  unsigned char r = b & 7;
  return (unsigned char)(((a << r) | (a >> (8 - r))) & 0xFF);
}

unsigned char hamming74_encode(unsigned char nibble) {
  unsigned char d0 = (nibble >> 0) & 1; unsigned char d1 = (nibble >> 1) & 1;
  unsigned char d2 = (nibble >> 2) & 1; unsigned char d3 = (nibble >> 3) & 1;

  unsigned char p0 = d0 ^ d1 ^ d3; unsigned char p1 = d0 ^ d2 ^ d3;
  unsigned char p2 = d1 ^ d2 ^ d3;

  return (p0 << 6) | (p1 << 5) | (d3 << 4) | (p2 << 3) | (d2 << 2) | (d1 << 1) | d0;
}

void embed_steg_hamming(unsigned char *block, int len, unsigned char control_byte) {
  if (len < 14) return;
  unsigned char high = (control_byte >> 4) & 0x0F;
  unsigned char low = control_byte & 0x0F;
  unsigned char code_high = hamming74_encode(high);
  unsigned char code_low = hamming74_encode(low);

  for (int i = 0; i < 7 && i < len; i++) {
    block[i] = (block[i] & 0xFE) | ((code_high >> (6 - i)) & 1);
  }
  for (int i = 0; i < 7 && (7 + i) < len; i++) {
    block[7 + i] = (block[7 + i] & 0xFE) | ((code_low >> (6 - i)) & 1);
  }
}

void morphing_encrypt_skip_rounds(unsigned char *block, unsigned char *key, MorphParams *params, unsigned int entropy, int key_len) {
  if (!block || !key || key_len <= 0) return;
  int rounds = NUM_ROUNDS_DEFAULT;

  for (int r = 0; r < rounds; r++) {
    if ((entropy & 0xFF) < 128 && (r % 3 == 0)) continue;

    for (int i = 0; i < key_len; i++) {
      unsigned char val = block[i];
      unsigned char k = key[(r + i) % key_len];

      switch (params->ops[0]) { case XOR_OP: val = op_xor(val, k); break; case ADD_OP: val = op_add(val, k); break; case ROTL_OP: val = op_rotl(val, k); break; }
      switch (params->ops[1]) { case XOR_OP: val = op_xor(val, k); break; case ADD_OP: val = op_add(val, k); break; case ROTL_OP: val = op_rotl(val, k); break; }
      switch (params->ops[2]) { case XOR_OP: val = op_xor(val, k); break; case ADD_OP: val = op_add(val, k); break; case ROTL_OP: val = op_rotl(val, k); break; }

      block[i] = val;
    }
    for (int i = 0; i < key_len; i++) key[i] = op_xor(key[i], block[i]);
  }
}

void generate_nested_chaos_freq_seq(unsigned int *freq_seq, unsigned char *block, int block_size, unsigned int entropy, unsigned char tda_val) {
  if (!freq_seq || !block || block_size <= 0) return;

  for (int i = 0; i < HOP_LEN; i++) {
    unsigned int base_freq = 2400 + (i * 3);
    unsigned int primary_offset = ((block[i % block_size] ^ ((entropy >> ((i % 4) * 8)) & 0xFF)) + tda_val * i) % 80;
    unsigned int micro_offset = (block[i % block_size] ^ (entropy & 0x1F)) % 6;
    freq_seq[i] = base_freq + primary_offset + micro_offset;
  }
}

void chaotic_delay(unsigned char *block, unsigned int entropy) {
  logistic_x = LOGISTIC_R * logistic_x * (1.0 - logistic_x);
  unsigned int chaos_factor = (unsigned int)(logistic_x * 16.0);
  delay(channelDelayBase + chaos_factor);
}

bool wake_up_trigger() {
  unsigned char tda_val = tda_feature_extraction();
  return tda_val > 30;
}

void rf_transmit(unsigned int freq, unsigned char *data, int len) {
  static int currentWifiChannel = 0;
  static int currentBtChannel = 0;

  if (currentMode == WIFI_JAMMING) {
    wifiJammer.setChannel(wifiChannels[currentWifiChannel % 11]);
    wifiJammer.write(data, len);
    currentWifiChannel = (currentWifiChannel + 1) % 11;
  } else {
    btJammer.setChannel(btChannels[currentBtChannel % 79]);
    btJammer.write(data, len);
    currentBtChannel = (currentBtChannel + 1) % 79;
  }
}

void innovative_chaos_jammer_final(unsigned char *master_key, unsigned char *payload, int length, unsigned long loop_index) {
  if (!wake_up_trigger()) {
    Serial.print("SKIP,"); Serial.print(loop_index);
    Serial.println(",0,0,0,0,0,0,0");
    return;
  }
  if (!master_key || !payload || length <= 0) return;

  int blocks = (length + BLOCK_SIZE - 1) / BLOCK_SIZE;
  MorphParams morph_params;
  unsigned int freq_seq[HOP_LEN];
  unsigned int entropy;
  unsigned char tda_feat;

  for (int i = 0; i < NOISE_HISTORY_LEN; i++) update_noise_history();

  for (int i = 0; i < blocks; i++) {
    int key_len;
    unsigned char block[BLOCK_SIZE];
    unsigned char key[20];

    entropy = fused_entropy_source();
    tda_feat = tda_feature_extraction();
    adaptive_morph_params(&morph_params, tda_feat);
    key_len = dynamic_key_length(entropy);

    Serial.print(currentMode == WIFI_JAMMING ? "WIFI," : "BT,");
    Serial.print(loop_index); Serial.print(",");
    Serial.print(i); Serial.print(",");
    Serial.print(tda_feat); Serial.print(",");
    Serial.print(entropy); Serial.print(",");
    Serial.print(key_len); Serial.print(",");
    Serial.print(morph_params.ops[0]); Serial.print(",");
    Serial.print(morph_params.ops[1]);
    Serial.println(morph_params.ops[2]);

    for (int j = 0; j < key_len; j++) key[j] = (j < BLOCK_SIZE) ? master_key[j] : ((unsigned char)(entropy & 0xFF) ^ j);
    for (int j = 0; j < BLOCK_SIZE; j++) block[j] = (i * BLOCK_SIZE + j < length) ? payload[i * BLOCK_SIZE + j] : 0;

    morphing_encrypt_skip_rounds(block, key, &morph_params, entropy, key_len);
    generate_nested_chaos_freq_seq(freq_seq, block, BLOCK_SIZE, entropy, tda_feat);
    embed_steg_hamming(block, BLOCK_SIZE, (unsigned char)(i & 0xFF));

    for (int h = 0; h < HOP_LEN; h++) {
      rf_transmit(freq_seq[h], block, BLOCK_SIZE);
      chaotic_delay(block, entropy);
    }
  }
}

void run_test_mode(unsigned char *master_key, unsigned char *payload, int length, unsigned long test_run) {
  if (!master_key || !payload || length <= 0) return;

  int blocks = (length + BLOCK_SIZE - 1) / BLOCK_SIZE;
  MorphParams morph_params;
  unsigned int entropy;
  unsigned char tda_feat;
  unsigned char temp_block[BLOCK_SIZE];

  for (int i = 0; i < NOISE_HISTORY_LEN; i++) update_noise_history();

  for (int i = 0; i < blocks; i++) {
    int key_len;
    unsigned char key[20];

    entropy = fused_entropy_source();
    tda_feat = tda_feature_extraction();
    adaptive_morph_params(&morph_params, tda_feat);
    key_len = dynamic_key_length(entropy);

    Serial.print("TEST,"); Serial.print(test_run); Serial.print(",");
    Serial.print(i); Serial.print(",");
    Serial.print(tda_feat); Serial.print(",");
    Serial.print(entropy); Serial.print(",");
    Serial.print(key_len); Serial.print(",");
    Serial.print(morph_params.ops[0]); Serial.print(",");
    Serial.print(morph_params.ops[1]);
    Serial.println(morph_params.ops[2]);

    for (int j = 0; j < key_len; j++) key[j] = (j < BLOCK_SIZE) ? master_key[j] : ((unsigned char)(entropy & 0xFF) ^ j);
    for (int j = 0; j < BLOCK_SIZE; j++) temp_block[j] = (i * BLOCK_SIZE + j < length) ? payload[i * BLOCK_SIZE + j] : 0;

    morphing_encrypt_skip_rounds(temp_block, key, &morph_params, entropy, key_len);
    embed_steg_hamming(temp_block, BLOCK_SIZE, (unsigned char)(i & 0xFF));
  }
}

void generateRandomPayload() {
  for (int i = 24; i < 32; i++) {
    interferencePayload[i] = random(32, 127);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== Advanced Multi-domain JAMMER (Research Mode) Initialized ===");
  Serial.println("TYPE,LOOP_INDEX,BLOCK_INDEX,TDA,ENTROPY,KEY_LEN,OP1,OP2,OP3");

  if (!wifiJammer.begin() || !btJammer.begin()) {
    Serial.println("WARNING: RF modules not detected. Running in computational TEST MODE only.");
  }

  wifiJammer.setPALevel(RF24_PA_HIGH);
  wifiJammer.stopListening();
  btJammer.setPALevel(RF24_PA_HIGH);
  btJammer.stopListening();

  randomSeed(analogRead(A0));
  modeStartMillis = millis();
}

unsigned long loop_counter = 0;

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - modeStartMillis > modeDuration) {
    currentMode = (currentMode == WIFI_JAMMING) ? BT_JAMMING : WIFI_JAMMING;
    modeStartMillis = currentMillis;
  }

  generateRandomPayload();

  unsigned char master_key[BLOCK_SIZE] = {
    0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x77, 0x88,
    0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11
  };

  run_test_mode(master_key, interferencePayload, sizeof(interferencePayload), loop_counter);
  loop_counter++;
  delay(100);
}
