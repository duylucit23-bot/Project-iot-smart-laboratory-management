# IoT-Based Smart Security & Environmental Monitoring System

## 📌 Project Overview
This repository contains the documentation and source code architecture for an Internet of Things (IoT) project focused on laboratory management. The system provides a decentralized, dual-factor authentication access control mechanism and real-time environmental telemetry, ensuring a secure and optimal climate for academic facilities.

## 🎯 Key Features & Objectives
* **Decentralized Hardware Architecture:** Strategically divided tasks between two microcontrollers to prevent electrical overload and ensure zero downtime.
* **Dual-Factor Access Control (Node 1):** Utilizes an ESP32 microcontroller to manage RFID (RC522) and PIN code (4x4 Keypad) authentication, actuating a physical servo barrier and providing LCD feedback.
* **Environmental Telemetry (Node 2):** Employs a Wemos D1 R2 (ESP8266) for single-threaded environmental monitoring, tracking temperature/humidity (DHT11) and pedestrian movement (PIR Sensor).
* **Automated Climate Intervention:** Features a closed-loop system where high temperature readings automatically trigger a 5V relay to activate a cooling fan.

## 🛠️ Technologies & Protocols
* **Microcontrollers:** ESP32, Wemos D1 R2 (ESP8266).
* **Sensors & Actuators:** RFID RC522, PIR Motion Sensor, DHT11, 4x4 Matrix Keypad, I2C LCD 1602, SG90 Servo Motor, 5V Relay.
* **Communication Protocol:** MQTT (via Eclipse Paho & PubSubClient libraries).
* **Backend Processing:** Custom Java Desktop Application utilizing Google GSON for JSON parsing and automated rules execution.

## 📂 Repository Structure
* `[CSN302-Report-Paper].doc` - The main academic paper detailing the system architecture, hardware design, and implementation results[cite: 3].
* `[Decentralized_Smart_Facility_Management].pdf` - The presentation slides illustrating the network topology, data flow, and closed-loop automation arc.
* `[Node 1 ESP32]/[Node 2 Wemos]` - The Arduino (C/C++) sketches for the edge nodes and the Java backend application source code.

## ⚙️ Data Flow & Logging
The system bypasses heavy relational databases by utilizing a highly efficient local logging mechanism. Both edge nodes continuously publish lightweight JSON strings over local Wi-Fi to an MQTT Broker. The centralized Java application subscribes to these topics, instantly capturing the data to execute automation rules and continuously appending events (with timestamps) to a local `.txt` log file.

---
*This project was completed as part of the academic curriculum at the School of Computing and Information Technology, Eastern International University.*
