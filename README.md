# 🚀 Adaptive Multi-Domain Chaotic Jammer  
### with Morphing Encryption, Entropy Harvesting & Steganographic Control

<p align="center">
  <img src="https://img.shields.io/badge/License-Apache%202.0-blue.svg?style=for-the-badge" alt="License">
  <img src="https://img.shields.io/badge/Status-Patent%20Published-success?style=for-the-badge" alt="Patent">
  <img src="https://img.shields.io/badge/Platform-ESP32%20%2B%20nRF24L01-orange?style=for-the-badge" alt="Platform">
  <img src="https://img.shields.io/badge/Domain-WiFi%20%26%20Bluetooth-purple?style=for-the-badge" alt="Domain">
  <img src="https://img.shields.io/badge/Research-Mode-red?style=for-the-badge" alt="Research">
  <img src="https://img.shields.io/badge/Firmware-Arduino-yellow?style=for-the-badge" alt="">
  <img src="https://img.shields.io/badge/Analysis-MATLAB-blue?style=for-the-badge" alt="MATLAB">
</p>

<p align="center">
  <b>A novel, patent-protected adaptive RF interference system</b><br>
  combining real-time entropy fusion, topological data analysis (TDA),  
  chaos-driven frequency hopping, and morphing encryption for robust,  
  low-detectability multi-domain jamming.
</p>

---

## 📖 Project Overview

This repository presents a complete **hardware + software research prototype** of an advanced multi-domain jammer that intelligently switches between Wi-Fi (2.4 GHz) and Bluetooth bands.  

Key innovations include:

- **Fused Entropy Source** – Radio noise + timing jitter  
- **Topological Data Analysis (TDA)** feature extraction for adaptive decisions  
- **Adaptive Morphing Encryption** – dynamic operator sequences (XOR / ADD / ROTL)  
- **Skip-Round Chaos Encryption** for variable diffusion  
- **Hamming(7,4) Steganographic Embedding** of control information  
- **Nested Chaos Frequency Hopping** with logistic-map timing  
- **Real-time Mode Switching** between Wi-Fi and Bluetooth jamming  

The system is designed for **research and evaluation purposes only**.

> ⚠️ **Patent Notice**  
> This work is protected under a **published patent** filed under the inventor’s name.  
> Commercial use, reproduction, or derivative works require appropriate licensing.

---

## 🎥 Demo Video

> **[🔗 Output / Demonstration Video – Google Drive Link (Placeholder)]**  
> *Replace this text with your actual Drive shareable link once the video is uploaded.*

---

## 🏗️ System Architecture & Workflow

The device follows a continuous closed-loop pipeline:

<p align="center">
  <img src="images/Device_Workflow.png" alt="Device Workflow" width="900">
</p>

**Figure 1 – High-level operational workflow**  
From power-up and interference mode selection, through entropy generation, channel analysis, payload creation, adaptive morphing encryption, error-control + steganographic embedding, chaos-based frequency hopping, RF transmission with chaotic timing, and continuous mode switching.

---

## 🔌 Hardware Implementation

### Circuit Diagram

<p align="center">
  <img src="images/Circuit_Diagram.jpeg" alt="Circuit Diagram" width="750">
</p>

**Figure 2 – Schematic of the dual nRF24L01 + ESP32-based prototype**  
Two independent nRF24L01 modules handle Wi-Fi and Bluetooth band coverage. Supporting components include power regulation, status LED, and battery.

### Physical Prototype

<p align="center">
  <img src="images/Hardware.png" alt="Hardware Prototype" width="700">
</p>

**Figure 3 – Assembled hardware**  
ESP32 microcontroller, dual nRF24L01 modules with antennas, breadboard interconnects, lithium battery, and status indicators.

---

## 🔐 Cryptographic & Diffusion Performance

### Bit-Flip Sensitivity (Avalanche Detail)

<p align="center">
  <img src="images/Bit-Flip%20Sensitivity%20Map%20(Avalanche%20Detail).png" alt="Bit-Flip Sensitivity Map" width="700">
</p>

**Figure 4 – Bit-Flip Sensitivity Map**  
Visualizes the avalanche effect: each plaintext bit position (x-axis) and its influence on ciphertext bits (y-axis). Bright regions indicate strong diffusion.

### Bit-Level Avalanche Effect Across Blocks

<p align="center">
  <img src="images/Bit-Level%20Avalanche%20Effect%20Across%20Blocks.png" alt="Bit-Level Avalanche" width="650">
</p>

**Figure 5 – Avalanche ratio per block**  
Shows how a single-bit change propagates across successive 16-byte blocks (ideal target ≈ 0.5).

### Byte-Level Diffusion

<p align="center">
  <img src="images/Byte-Level%20Diffusion%20per%20Block.png" alt="Byte-Level Diffusion" width="650">
</p>

**Figure 6 – Byte-level diffusion ratio**  
Demonstrates progressive improvement in diffusion strength across blocks.

### Round-by-Round Diffusion in Morphing Encryption

<p align="center">
  <img src="images/Round-by-Round%20Diffusion%20in%20Morphing%20Encryption.png" alt="Round-by-Round Diffusion" width="700">
</p>

**Figure 7 – Diffusion evolution over rounds**  
Heatmap illustrating how the adaptive morphing cipher rapidly achieves high diffusion (yellow = strong) within the first few rounds.

### Entropy Gain After Adaptive Morphing Encryption

<p align="center">
  <img src="images/Entropy%20Gain%20After%20Adaptive%20Morphing%20Encryption.png" alt="Entropy Gain" width="700">
</p>

**Figure 8 – Shannon entropy comparison**  
Plaintext (blue) vs. ciphertext (orange) entropy per block, confirming significant entropy increase after encryption.

### Fused Entropy Source Behavior

<p align="center">
  <img src="images/Fused%20Entropy%20Source%20vs.%20Block.png" alt="Fused Entropy" width="650">
</p>

**Figure 9 – Normalized fused entropy across blocks**  
Shows the dynamic entropy harvested from radio noise and timing jitter that drives key length, morph parameters, and hopping.

### Time-Domain Ciphertext Randomness

<p align="center">
  <img src="images/Time-Domain%20Ciphertext%20Randomness.png" alt="Time-Domain Randomness" width="700">
</p>

**Figure 10 – Ciphertext byte stream in time domain**  
Highly irregular amplitude patterns confirm effective randomization.

### Frequency-Domain Spectrum of Ciphertext

<p align="center">
  <img src="images/Frequency-Domain%20Spectrum%20of%20Ciphertext%20Stream.png" alt="Frequency Spectrum" width="650">
</p>

**Figure 11 – Frequency-domain spectrum**  
Near-flat spectrum (except low-frequency residual) indicates good noise-like properties of the ciphertext stream.

---

## 🧠 Core Algorithm Highlights

| Component                      | Description                                                                 |
|--------------------------------|-----------------------------------------------------------------------------|
| **Fused Entropy Source**       | Radio noise XOR timing jitter + bit mixing                                 |
| **TDA Feature Extraction**     | Counts bit flips in noise history buffer for adaptive decisions            |
| **Dynamic Key Length**         | 12–20 bytes based on entropy                                               |
| **Adaptive Morph Parameters**  | Operator order (XOR / ADD / ROTL) selected by TDA value                    |
| **Skip-Round Encryption**      | Randomly skips rounds based on entropy for variable diffusion              |
| **Hamming(7,4) Stego**         | Embeds control byte into LSBs of ciphertext using error-correcting codes   |
| **Nested Chaos Hopping**       | Logistic map + ciphertext-driven frequency offsets                         |
| **Chaotic Timing**             | Variable inter-hop delays driven by logistic map                           |

---

## 🛠️ Getting Started

### Hardware Requirements
- ESP32 development board
- 2 × nRF24L01(+PA+LNA recommended) modules
- Dual antennas
- 3.7 V Li-ion battery + charger module
- Breadboard / custom PCB
- Status LED (optional)

### Software Setup
1. Install Arduino IDE or PlatformIO
2. Install libraries: `RF24`, `nRF24L01`, `SPI`
3. Select ESP32 board and correct COM port
4. Upload the provided sketch
5. Open Serial Monitor at **115200 baud**

### Serial Output Format
```
TYPE,LOOP_INDEX,BLOCK_INDEX,TDA,ENTROPY,KEY_LEN,OP1,OP2,OP3
```
- `TYPE` → `WIFI`, `BT`, `TEST`, or `SKIP`
- Real-time telemetry of adaptive decisions

---

## 📜 License

This project is released under the **Apache License 2.0**.

```
Copyright 2025 [Your Name / Inventor]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

---

## 🛡️ Patent & Intellectual Property

This system and its core methods are the subject of a **published patent** under the inventor’s name.  

**All rights reserved.**  
Unauthorized commercial exploitation, reverse engineering for product development, or derivative commercial systems are prohibited without a formal license agreement.

For licensing inquiries, please contact the patent holder.

---

## ⚠️ Legal & Ethical Notice

This repository is provided **strictly for academic, research, and educational purposes**.  
The authors and patent holder accept **no responsibility** for any misuse of the technology.  
Users must comply with all applicable local, national, and international laws regarding radio spectrum usage and interference.

---

## 🌟 Acknowledgments

- Built on the robust `RF24` library ecosystem  
- Inspired by research in chaotic cryptography, topological data analysis, and adaptive RF systems  

---

<p align="center">
  <b>⭐ If this research inspires you, consider starring the repository!</b><br><br>
  <i>Adaptive • Chaotic • Morphing • Multi-Domain</i>
</p>
