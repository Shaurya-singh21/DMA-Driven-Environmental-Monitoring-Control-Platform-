# Sentinel-M4: DMA-Driven Thermal Governor & Security Monitor 🛡️❄️

## Project Overview
Sentinel-M4 is a high-precision environmental control and security system built on the **STM32F446RE (Cortex-M4)**. It differentiates itself from standard hobbyist projects by utilizing **bare-metal register access** and **Level 3 DMA pipelines** to achieve real-time responsiveness with zero-CPU overhead sensor acquisition.

The system manages a stable "Thermal Zone" (30°C–32°C) while simultaneously monitoring for security anomalies using a **30-20-10-1 Artificial Neural Network (ANN)** trained in PyTorch.

---

## 🛠️ Hardware Architecture
* **MCU**: STM32F446RE (180MHz max clock).
* **Power**: MP1584 Buck Converter tuned to 5V with 470µF bulk decoupling to suppress motor spikes.
* **Sensors**: 
  * **NTC Thermistor**: High-sensitivity thermal monitoring.
  * **LDR (Light Dependent Resistor)**: Light-based breach detection.
  * **IR Proximity**: Safety interlock for fan blockage detection.
* **Actuators**:
  * **DC Fan**: Driven by L9110S H-Bridge via 20kHz Silent PWM.
  * **Servo Motor**: MG90S managing physical venting via 50Hz PWM.



---

## 💻 Firmware Features (Register-Level)
* **DMA Acquisition Pipeline**: Uses `DMA2_Stream0` to offload ADC sampling. CPU only wakes to process averaged results, maximizing efficiency.
* **Advanced Timer Control**:
  * **TIM1**: Configured for 20kHz ultrasonic PWM to eliminate motor whine.
  * **TIM3**: High-precision TRGO trigger for the ADC sampling sequence.
  * **TIM4**: Dedicated 50Hz servo control with fine-tuned pulse width calibration.
* **I2C OLED Dashboard**: DMA-driven framebuffer flush (SSD1306) to ensure UI updates never block the real-time control loop.
* **Predictive Edge AI**: Logs CSV telemetry via UART to train an ANN for anomaly detection (Breach vs. Ambient Shift).

---

## 🚦 Operational Logic (Hysteresis Window)
| State | Temp Threshold | Action |
| :--- | :--- | :--- |
| **Idle** | < 30.0°C | Fan OFF, Vent CLOSED. |
| **Active Cooling**| 31.5°C - 34.0°C | Vent OPENS, Fan ramps PWM based on thermal gradient. |
| **Critical/Breach**| > 34.0°C | Fan 100%, LED Blink (TIM2), UART Alert. |



---

## 📂 Repository Structure
* `/src/main.c`: Core state machine and threshold logic.
* `/src/dma.c`: Interrupt-driven data transfer for ADC/I2C.
* `/src/timers.c`: PWM and TRGO clock configurations.
* `/src/adc.c`: 12-bit multi-channel scan mode configuration.
* `/src/oled.c`: SSD1306 driver with character mapping and DMA-flush.

---

## 🚀 Future Roadmap
* **Temporal Context**: Integrating the internal **RTC** to distinguish between daytime ambient rise and midnight breaches.
* **On-Chip Inference**: Porting the PyTorch ANN to **CMSIS-NN** for fully autonomous on-chip anomaly detection.

---
Developed as a demonstration of **Embedded Systems Architecture** and **Real-Time Control Loops**.
