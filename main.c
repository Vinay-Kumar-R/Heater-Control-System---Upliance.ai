#include <OneWire.h>
#include <DallasTemperature.h>
// Header files in case we want to implement BLE on the physical board
// #include <BLEDevice.h>
// #include <BLEServer.h>
// #include <BLEUtils.h>
// #include <BLE2902.h>

// --- Pin Definitions ---
#define ONE_WIRE_BUS 16 // DS18B20 data pin connected to GPIO16
#define HEATER_PIN 4    // LED connected to simulate heater on GPIO4
#define STATUS_LED_PIN 15 // For a status LED on GPIO15 (if implementing bonus)
#define STATUS_BUZZER_PIN 2 // For a status Buzzer on GPIO2 (if implementing bonus)

// --- Temperature Thresholds and System Parameters ---
const float TARGET_TEMP = 30.0;       // Example target temperature in Celsius
const float HEATING_THRESHOLD_LOW = 28.0; // Heater turns ON below this temperature
const float HEATING_THRESHOLD_HIGH = 31.0; // Heater turns OFF above this (simple hysteresis to prevent rapid on/off)
const float OVERHEAT_THRESHOLD = 35.0; // Critical temperature for OVERHEAT state

// --- DS18B20 Sensor Setup ---
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// --- System State Definition ---
enum SystemState {
  IDLE,
  HEATING,
  STABILIZING,      // For a period where temp is near target but not fully stable
  TARGET_REACHED,
  OVERHEAT
};

SystemState currentState = IDLE; // Initial state

unsigned long lastTempReadTime = 0;
const unsigned long TEMP_READ_INTERVAL_MS = 2000; // Read temperature every 2 seconds

// --- Function Prototypes (Declare functions before they are used) ---
float readTemperature();
void updateSystemState(float currentTemp);
void controlHeater(SystemState state);
void logStatus(float temp);
void updateStatusLed(SystemState state); // For bonus LED indicator
void updateBuzzerStatus(SystemState state);
// void setupBLE(); // For BLE advertising
// void updateBLEAdvertising(); // For BLE advertising

// --- Arduino Setup Function ---
void setup() {
  Serial.begin(115200); // Initialize Serial communication for logging
  Serial.println("Heater Control System Starting...");

  sensors.begin(); // Initialize DS18B20 sensor
  sensors.setResolution(10); // Set sensor resolution (e.g., 9, 10, 11, or 12 bits)

  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW); // Ensure heater is off initially

  // Optional: For a status LED/Buzzer
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  pinMode(STATUS_BUZZER_PIN, OUTPUT); // Ensure buzzer pin is set as output
  digitalWrite(STATUS_BUZZER_PIN, LOW); // Ensure buzzer is off initially
  // For BLE advertising
  // setupBLE(); // Call BLE setup function
}

// --- Arduino Loop Function ---
void loop() {
  unsigned long currentTime = millis();

  // Read temperature periodically to avoid blocking
  if (currentTime - lastTempReadTime >= TEMP_READ_INTERVAL_MS) {
    float currentTemp = readTemperature();
    if (currentTemp != -999.0) { // Only proceed if temperature read was successful
      updateSystemState(currentTemp);
      controlHeater(currentState);
      logStatus(currentTemp);
      updateStatusLed(currentState);
      updateBuzzerStatus(currentState);
      // updateBLEAdvertising();
    }
    lastTempReadTime = currentTime;
  }
  
}

// --- Helper Functions ---

/**
 * @brief Reads temperature from the DS18B20 sensor.
 * @return Current temperature in Celsius, or -999.0 on error.
 */
float readTemperature() {
  sensors.requestTemperatures(); // Send the command to get temperatures
  float tempC = sensors.getTempCByIndex(0); // Get temperature from the first sensor (index 0)

  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: Could not read temperature sensor!");
    return -999.0; // Return an error code
  }
  return tempC;
}

/**
 * @brief Updates the system's state based on current temperature.
 * @param currentTemp The current temperature reading.
 */
void updateSystemState(float currentTemp) {
  SystemState previousState = currentState; // Store previous state for logging state changes

  switch (currentState) {
    case IDLE:
      if (currentTemp < HEATING_THRESHOLD_LOW) {
        currentState = HEATING;
      } else if (currentTemp >= OVERHEAT_THRESHOLD) {
        currentState = OVERHEAT;
      }
      break;

    case HEATING:
      if (currentTemp >= HEATING_THRESHOLD_HIGH && currentTemp < OVERHEAT_THRESHOLD) {
        currentState = STABILIZING; // Transition to stabilizing near target
      } else if (currentTemp >= OVERHEAT_THRESHOLD) {
        currentState = OVERHEAT; // Critical overheat detected
      }
      break;

    case STABILIZING:
      // In a real system, you might add a timer here to ensure stability for a duration.
      // For simplicity, we check if it falls out of range or reaches target.
      if (currentTemp < HEATING_THRESHOLD_LOW) {
        currentState = HEATING; // Temperature dropped significantly
      } else if (currentTemp >= OVERHEAT_THRESHOLD) {
        currentState = OVERHEAT; // Overheat during stabilizing
      } else if (currentTemp >= TARGET_TEMP - 0.5 && currentTemp <= TARGET_TEMP + 0.5) { // Within a small window around target
        currentState = TARGET_REACHED;
      }
      break;

    case TARGET_REACHED:
      if (currentTemp < HEATING_THRESHOLD_LOW) {
        currentState = HEATING; // Temperature dropped, need to reheat
      } else if (currentTemp >= OVERHEAT_THRESHOLD) {
        currentState = OVERHEAT; // Overheat after reaching target
      }
      break;

    case OVERHEAT:
      // Safety First: Heater must be OFF.
      // System stays in OVERHEAT until temperature drops sufficiently.
      if (currentTemp < (OVERHEAT_THRESHOLD - 2.0)) { // Example: drops 2 degrees below overheat threshold
        currentState = IDLE; // Return to IDLE or HEATING if still below target
      }
      break;
  }

  if (currentState != previousState) {
    Serial.print("State change: ");
    Serial.print(previousState == IDLE ? "IDLE" : (previousState == HEATING ? "HEATING" : (previousState == STABILIZING ? "STABILIZING" : (previousState == TARGET_REACHED ? "TARGET_REACHED" : "OVERHEAT"))));
    Serial.print(" -> ");
    Serial.println(currentState == IDLE ? "IDLE" : (currentState == HEATING ? "HEATING" : (currentState == STABILIZING ? "STABILIZING" : (currentState == TARGET_REACHED ? "TARGET_REACHED" : "OVERHEAT"))));
  }
}

/**
 * @brief Controls the heater (simulated by an LED) based on the current system state.
 * @param state The current SystemState.
 */
void controlHeater(SystemState state) {
  // Heater is ON only in HEATING state
  if (state == HEATING) {
    digitalWrite(HEATER_PIN, HIGH); // Turn heater ON
  } else {
    digitalWrite(HEATER_PIN, LOW); // Turn heater OFF for all other states (IDLE, STABILIZING, TARGET_REACHED, OVERHEAT)
  }

  // Ensure heater is OFF if in OVERHEAT state as a critical safety measure
  if (state == OVERHEAT) {
    digitalWrite(HEATER_PIN, LOW);
    // Potentially activate a separate critical alarm/buzzer here
  }
}

/**
 * @brief Logs the system's current status to the Serial Monitor.
 * @param temp The current temperature reading.
 */
void logStatus(float temp) {
  Serial.print("[");
  Serial.print(millis()); // Log time since start
  Serial.print("ms] Temp: ");
  Serial.print(temp, 1); // Print temperature with 1 decimal place
  Serial.print("C, State: ");
  // Print current state name
  switch (currentState) {
    case IDLE: Serial.print("IDLE"); break;
    case HEATING: Serial.print("HEATING"); break;
    case STABILIZING: Serial.print("STABILIZING"); break;
    case TARGET_REACHED: Serial.print("TARGET_REACHED"); break;
    case OVERHEAT: Serial.print("OVERHEAT"); break;
  }
  Serial.print(", Heater: ");
  Serial.println(digitalRead(HEATER_PIN) == HIGH ? "ON" : "OFF");
}
// --- Helper Functions (continued) ---

// Static variables to keep track of the last toggle time for blinking
static unsigned long lastLedToggleTime = 0;

/**
 * @brief Controls the status LED based on the current system state.
 * @param state The current SystemState.
 */
void updateStatusLed(SystemState state) {
  unsigned long currentTime = millis(); // Get current time for non-blocking delays

  // Define blinking intervals for different states
  const unsigned long HEATING_BLINK_INTERVAL = 75;     // Fast blink
  const unsigned long STABILIZING_BLINK_INTERVAL = 200; // Medium blink
  const unsigned long OVERHEAT_BLINK_INTERVAL =38;    // Very fast blink (for urgency)

  switch (state) {
    case IDLE:
      digitalWrite(STATUS_LED_PIN, LOW); // LED is OFF when idle
      break;

    case HEATING:
      // Fast blink: toggle LED if enough time has passed
      if (currentTime - lastLedToggleTime >= HEATING_BLINK_INTERVAL) {
        digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN)); // Toggle current state
        lastLedToggleTime = currentTime; // Update last toggle time
      }
      break;

    case STABILIZING:
      // Medium blink: toggle LED if enough time has passed
      if (currentTime - lastLedToggleTime >= STABILIZING_BLINK_INTERVAL) {
        digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN)); // Toggle current state
        lastLedToggleTime = currentTime;
      }
      break;

    case TARGET_REACHED:
      digitalWrite(STATUS_LED_PIN, HIGH); // LED is SOLID ON when target reached
      break;

    case OVERHEAT:
      // Very fast blink: for critical warning
      if (currentTime - lastLedToggleTime >= OVERHEAT_BLINK_INTERVAL) {
        digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN)); // Toggle current state
        lastLedToggleTime = currentTime;
      }
      break;
  }
}

void updateBuzzerStatus(SystemState state) {
  if (state == OVERHEAT) {
    // Play a continuous tone for overheat.
    // tone(pin, frequency, duration). If duration is 0, it plays indefinitely until noTone().
    tone(STATUS_BUZZER_PIN, 10000); // Play a 1000 Hz tone
  } else {
    // In any other state, ensure the buzzer is off.
    noTone(STATUS_BUZZER_PIN); // Stop any ongoing tone
    digitalWrite(STATUS_BUZZER_PIN, LOW); // Ensure the pin is low for active buzzers
  }
}

/*
// --- BLE Service and Characteristic UUIDs (Generate your own UUIDs) ---
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b" // Example Service UUID
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8" // Example Characteristic UUID

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false; // Flag to track if a BLE client is connected

// Callbacks for BLE server events
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("BLE Client Connected");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("BLE Client Disconnected");
      // Restart advertising to allow new connections
      pServer->startAdvertising();
    }
};

// @brief Optional: Sets up BLE advertising for the ESP32.
void setupBLE() {
  BLEDevice::init("HeaterControl_upliance_Vinay"); // Name of your BLE device
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // helps with iPhone connection issues
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("BLE Advertising started...");
}

// @brief Optional: Updates the BLE characteristic with the current heating state.
void updateBLEAdvertising() {
  if (deviceConnected) {
    String stateStr;
    switch (currentState) {
      case IDLE: stateStr = "IDLE"; break;
      case HEATING: stateStr = "HEATING"; break;
      case STABILIZING: stateStr = "STABILIZING"; break;
      case TARGET_REACHED: stateStr = "TARGET_REACHED"; break;
      case OVERHEAT: stateStr = "OVERHEAT"; break;
      default: stateStr = "UNKNOWN"; break;
    }
    pCharacteristic->setValue(stateStr.c_str());
    pCharacteristic->notify();
    Serial.print("BLE Advertised State: ");
    Serial.println(stateStr);
  }
}
*/
