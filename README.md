# Heater-Control-System---Upliance.ai
A repository containing my assignment given by Upliance.ai

# Heater Control System (Wokwi ESP32 Simulation)

## Project Overview

This repository contains the embedded C/C++ (Arduino framework) implementation for a basic heater control system, simulated on the Wokwi online ESP32 simulator. The project demonstrates core embedded systems principles, including sensor interaction, state-based logic, and actuator control, with visual and auditory feedback.

## Core Implementation

The system is designed to maintain a target temperature range by controlling a simulated heating element. It continuously monitors temperature and transitions through various operational states:

* **Temperature-Based State Tracker:** The system dynamically updates its state based on real-time temperature readings:
    * `IDLE`: System waiting; temperature within an acceptable range.
    * `HEATING`: Temperature is below the lower threshold; the heater is actively engaged.
    * `STABILIZING`: Temperature is approaching the target, the heater may be off as the system settles.
    * `TARGET_REACHED`: The desired temperature is achieved and maintained.
    * `OVERHEAT`: A critical safety state where the temperature exceeds predefined safe limits, and the heater is immediately forced off.
* **Continuous Temperature Monitoring:** Utilizes a DS18B20 temperature sensor to provide regular readings.
* **Heater Control:** Automatically turns the simulated heater (represented by an LED) on or off based on preset temperature thresholds, incorporating hysteresis to ensure stable operation and prevent rapid cycling.
* **Serial Logging:** Provides comprehensive status updates (current temperature, system state, heater status) to the Serial Monitor for real-time monitoring and debugging.

## Visual & Auditory Feedback

* **LED Indicator:** A dedicated status LED provides immediate visual feedback:
    * `IDLE`: LED Off
    * `HEATING`: Fast blinking
    * `STABILIZING`: Medium blinking
    * `TARGET_REACHED`: Solid On
    * `OVERHEAT`: Very fast blinking (critical alert)
* **Buzzer Alarm:** An audible buzzer activates specifically when the system enters the `OVERHEAT` state, providing an urgent alarm. The buzzer automatically stops once the temperature drops below the overheat threshold, indicating a return to a safer state.

## Bonus Features (Conceptual / Observational)

This project explores additional advanced concepts, providing the foundation for more complex embedded systems:

* **BLE (Bluetooth Low Energy) Advertising:**
    * **Concept:** The code includes the necessary structure to broadcast the system's current state (e.g., "IDLE", "HEATING") via BLE advertising packets.
    * **Wokwi Simulation:** While direct connection to a physical mobile phone is not possible in this online simulation environment, I believe that the code that I have given fully works and can be implemented on a physical ESP32 board. 
* **FreeRTOS Implementation:**
    * **Concept:** FreeRTOS is a powerful Real-Time Operating System (RTOS) that underlies the ESP32's Arduino framework. It offers significant benefits for managing concurrent operations, prioritizing tasks, and enhancing system responsiveness in more advanced embedded applications. While not fully implemented with multiple distinct tasks in this project's current scope due to its inherent complexity, the project design conceptually acknowledges how different parts of the system (e.g., sensor reading, control logic, communication) could be separated into independent FreeRTOS tasks for better modularity and real-time performance.

## Platform and Language

* **Platform:** Wokwi (Online ESP32 Simulator)
* **Language:** C/C++ (Arduino Framework)
