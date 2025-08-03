// So I made a prototype of a Motor speed simulator script where instead of a driver, I used a LED. This was useful in learning the basics of ESP32 board.

int motorPin = 18;

void setup() {
  pinMode(motorPin, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  for (int speed = 0; speed <= 255; speed++) {
    digitalWrite(motorPin, HIGH);
    Serial.println(speed);  // Channel 0, speed (0-255)
    delay(10);
  }
  for (int speed = 255; speed >= 0; speed--) {
    digitalWrite(motorPin, LOW);
    Serial.println(speed); 
    delay(10);
  }
}
