#include <Arduino.h>
#include <BleKeyboard.h>

BleKeyboard bleKeyboard("Mute O Matic", "ERK", 100);

#define MUTE 0
#define UNMUTE 1

const int PINS[2] = {33, 32};
const int KEYS[2] = {0x97, 0x98};

// Button states
int buttonState[2] = {0, 0};
int lastButtonState[2] = {0, 0};

unsigned long lastDebounceTime[2] = {0, 0};
unsigned long debounceDelay = 50;

// Pins
const int BATT = A13;
const int LED = 13;

// Battery
const float MAX_ADC = 4095;
const float REF_VOLTAGE = 3.3;
const float ADC_REF_VOLTAGE = 1.1;
unsigned long lastBatteryCheck = 0;
unsigned long batteryInterval = 30000;
uint8_t batteryPercentage;


void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  pinMode(LED, OUTPUT);

  bleKeyboard.begin();

  while(!bleKeyboard.isConnected()) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.println("Keyboard connected");
}

void sendKeys(int idx) {
  bleKeyboard.press(KEY_LEFT_ALT);
  bleKeyboard.press(KEYS[idx]);
  delay(50);
  bleKeyboard.releaseAll();
  delay(300);
}


void checkButton(int idx) {
  int reading = touchRead(PINS[idx]) < 10;

  if ((millis() - lastDebounceTime[idx]) > debounceDelay) {
    if (reading > 0) {
      Serial.print("press for ");
      Serial.print(idx);
      Serial.println();
      sendKeys(idx);
    }
    lastDebounceTime[idx] = millis();
  }
}

void checkBattery() {
  int reading = analogRead(BATT);
  float voltage = (reading / MAX_ADC) * 2.0 * REF_VOLTAGE * ADC_REF_VOLTAGE;
  batteryPercentage = map(voltage, 3.7, 4.2, 20, 100);

  if ((millis() - lastBatteryCheck) > batteryInterval) {
    lastBatteryCheck = millis();
    bleKeyboard.setBatteryLevel(batteryPercentage);
    Serial.print("Battery: ");
    Serial.print(batteryPercentage);
    Serial.println("%");
  }
}

void loop() {
  if(bleKeyboard.isConnected()) {
    digitalWrite(LED, HIGH);
    checkButton(MUTE);
    checkButton(UNMUTE);
    checkBattery();
  } else {
    digitalWrite(LED, LOW);
  }
}
