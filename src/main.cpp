#include <Arduino.h>
#include <BleKeyboard.h>

BleKeyboard bleKeyboard;

#define MUTE 0
#define UNMUTE 1

const int PINS[2] = {33, 32};
const int KEYS[2] = {0x97, 0x98};

int buttonState[2] = {0, 0};
int lastButtonState[2] = {0, 0};

unsigned long lastDebounceTime[2] = {0, 0};
unsigned long debounceDelay = 150;

const int BATT = A13;
const int LED = 13;


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
  delay(50);
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
  float voltage = map(reading, 0, 4095, 0.0, 4.7);
  Serial.println(reading);
  Serial.println(voltage);
}

void loop() {
  if(bleKeyboard.isConnected()) {
    digitalWrite(LED, HIGH);
    checkButton(MUTE);
    checkButton(UNMUTE);
  } else {
    digitalWrite(LED, LOW);
  }
}
