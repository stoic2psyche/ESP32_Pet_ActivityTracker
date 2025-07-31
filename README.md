# ESP32_Pet_ActivityTracker

This project is a lightweight, low-power dog activity tracker built using the **ESP32-S3 Dev Module**. It uses motion data to classify your pet's activity in real time and logs it with accurate timestamps for later review.

## 🚀 Features

- 🐕‍🦺 Real-time classification of dog activities (e.g., walking, resting, running)
- ⚙️ Simulated **QMI8658 IMU Sensor** for motion data
- 🧠 **TensorFlow Lite Micro** model for activity classification
- 🕒 **NTP-synchronized timestamping** using a custom `TimeManager` class
- 💾 **LittleFS**-based storage for logging activity data locally
- 🔋 Power-efficient design using:
  - Wake-on-motion detection
  - Deep sleep mode between events

## 📦 Tech Stack

- **Microcontroller:** ESP32-S3 Dev Module  
- **IDE:** Arduino IDE  
- **Libraries Used:**
  - TensorFlow Lite Micro
  - LittleFS
  - WiFi & NTP for time synchronization

## 🛠 Project Structure

```plaintext
├── ActivityClassifier.h   # Handles TensorFlow Lite model inference
├── ActivityLogger.h       # Manages data logging using LittleFS
├── TimeManager.h          # Syncs time via NTP and keeps RTC time
├── main.ino               # Core logic: setup, loop, motion handling
├── model.tflite           # Pretrained activity classification model
└── README.md              # Project documentation
