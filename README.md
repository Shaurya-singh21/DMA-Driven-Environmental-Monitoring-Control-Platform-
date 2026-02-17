# 🧊 DMA-Driven Environmental Monitoring & Control Platform 
### Register-Level STM32 Firmware for High-Reliability Enclosures

![STM32F446RE](https://img.shields.io/badge/MCU-STM32F446RE-blue?style=flat-square&logo=stmicroelectronics)
![Language](https://img.shields.io/badge/Language-Bare--Metal%20C-green?style=flat-square)
![Binary Size](https://img.shields.io/badge/Binary%20Size-~8KB-red?style=flat-square)

A high-performance climate control and security monitoring system built from the ground up with **Zero-HAL dependencies**. By utilizing **Direct Register Manipulation** across 9 peripherals, this system achieves deterministic real-time response with a 40% reduction in binary footprint (from 25KB to ~8KB).

---

## 🛠️ Hardware & Peripheral Architecture
The system is engineered for maximum CPU availability, offloading all data-heavy tasks to hardware-level pipelines.

### Peripheral Configuration:
* **TIM1 (DC Motor)**: Configured for **20kHz Silent PWM** to eliminate audible switching whine.
* **TIM3 (ADC Trigger)**: Acts as the master heartbeat, triggering **ADC1** every 1000ms via TRGO.
* **TIM4 (Servo)**: 50Hz Standard PWM for precise 1ms–2ms pulse-width modulation.
* **ADC1 (Sensor Fusion)**: 12-bit scan mode capturing NTC Thermistor and LDR data.
* **DMA2 (Stream 0)**: Manages **Circular ADC Acquisition**, moving raw samples directly to SRAM.
* **DMA1 (Stream 6)**: Asynchronous I2C flushes for the OLED framebuffer (1025 bytes).

## 📡 Communication Protocols

### 1. Interrupt-Driven USART Telemetry
To ensure the **Register-Level Thermal Governor** remains deterministic, the telemetry engine is strictly non-blocking. The system utilizes **USART2** (PA2) for real-time data logging.
* **Async State Machine**: The `send()` function initiates the transfer by setting a pointer to the data string and enabling the `TXEIE` (Transmit Data Register Empty) interrupt.
* **Non-Blocking ISR**: The `USART2_IRQHandler` handles byte-by-byte transmission. This prevents the CPU from stalling in "while-loops" during long string transfers, ensuring the cooling logic and ADC sampling are never delayed.
* **Data Payload**: Logs 8-column timestamped CSV telemetry (Baud: 115200) including: `CNT, TEMP, RAW_TEMP, LDR, RAW_LDR, DOOR, FAN, VENT`.

### 2. Hybrid DMA/Blocking I2C Interface
The system communicates with the **SSD1306 OLED** over **I2C1** (PB6:SCL, PB7:SDA) using a dual-mode driver optimized for the STM32 bus matrix.
* **Hardware Recovery Logic**: Implements a dedicated `i2c_reset()` routine that monitors the `BUSY` flag. If a bus hang is detected, it performs a peripheral software reset and reconfigures the `CCR` and `TRISE` registers to recover the link without a system reboot.
* **DMA Framebuffer Flush**: For high-bandwidth 1025-byte display updates, the system utilizes **DMA1 Stream 6 (Channel 1)**. This allows the entire display buffer to be pushed to the OLED with zero CPU intervention, maintaining high frame rates while the CPU executes control logic.
* **Deterministic Polling**: Uses low-latency blocking sends for critical initialization commands, ensuring the peripheral is correctly synchronized before the main control loop starts.



---

## 🚦 Control Theory & Logic
The governor implements a **Double-Threshold Hysteresis Window** to prevent actuator "hunting" and unnecessary power consumption.

| State | Temp Condition | Actuator Response |
| :--- | :--- | :--- |
| **Idle** | $30.0^\circ\text{C} - 31.5^\circ\text{C}$ | System monitoring; All actuators standby. |
| **Cooling** | $> 31.5^\circ\text{C}$ | Servo to **90°**, Fan PWM starts (TIM1->CCR2). |
| **Heating** | $< 30.0^\circ\text{C}$ | Servo to **45°** for air recirculation; Blink alert (TIM2). |
| **Breach** | LDR/IR Trigger | UART Alert + Emergency visual strobe. |



---

## 💻 Firmware Engineering Details

### 1. Level 3 DMA Acquisition Pipeline
Unlike typical implementations that waste CPU cycles waiting for ADC results, this project uses **DMA2 Stream 0** to autonomously fill a buffer. The CPU only wakes up when the **Transfer Complete (TC)** interrupt is fired, allowing for complex logic execution without sampling jitter.

### 2. Deterministic Thermal Modeling
Temperature is calculated using the **Steinhart-Hart Equation** for $\pm 0.5^\circ\text{C}$ precision:
$$\frac{1}{T} = A + B\ln(R) + C(\ln(R))^3$$
This is processed in the non-blocking main loop after the DMA buffer is ready.

### 3. Jitter-Free Telemetry
The **UART Log Engine** (115200 Baud) uses an interrupt-driven state machine to send timestamped CSV data without stalling the main control loop.

---

## 📂 File Breakdown
* **`main.c`**: Deterministic state machine and thermal governor logic.
* **`dma.c`**: Configuration for Peripheral-to-Memory and Memory-to-Peripheral streams.
* **`adc.c`**: Register settings for 12-bit resolution and TRGO synchronization.
* **`timers.c`**: PWM generation (20kHz & 50Hz) and mechanical position ramping.
* **`oled.c`**: Low-level SSD1306 driver with DMA framebuffer flush.

---

## 🔮 Roadmap
- [ ] **TinyML Integration**: Deploying an **ANN** to predict "Thermal Breaches" based on $dT/dt$ gradients.
- [ ] **RTC Synchronization**: Integrating the internal Real-Time Clock for temporal security logging.
- [ ] **CMSIS-DSP Optimization**: Using ARM-specific instructions to accelerate Steinhart-Hart floating-point calculations.

---
**Developed by [Your Name] | IIST | specialized in Low-Level Firmware Architecture**
