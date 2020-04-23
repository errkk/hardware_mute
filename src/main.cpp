#include <Arduino.h>
#include <BleKeyboard.h>

BleKeyboard bleKeyboard("Mute O Matic", "ERK", 100);

#define MUTE 1
#define UNMUTE 0

const int PINS[2] = {33, 32};
const int KEYS[2] = {0x97, 0x98};

// Button states
int buttonState[2] = {0, 0};
int lastButtonState[2] = {0, 0};
int buttonComplete[2] = {0, 0};

unsigned long lastDebounceTime[2] = {0, 0};
unsigned long debounceDelay = 50;
String label[2] = {"Unmute", "Mute"};

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
  Serial.println(label[idx]);
  bleKeyboard.press(KEY_LEFT_ALT);
  bleKeyboard.press(KEYS[idx]);
  delay(50);
  bleKeyboard.releaseAll();
  delay(300);
}


void checkButton(int idx) {
  int reading = touchRead(PINS[idx]) < 15;

  // Has it changed? Due to noise or pressing
  if (reading != lastButtonState[idx]) {
    lastDebounceTime[idx] = millis();
  }

  // Has it been in this state long enough?
  if ((millis() - lastDebounceTime[idx]) > debounceDelay) {

    // Has the signal changed?
    if (reading != buttonState[idx]) {
      buttonState[idx] = reading;

      // Is it a press
      if (reading > 0) {
        sendKeys(idx);
        buttonComplete[idx] = true;
      }
    }
  }
  lastButtonState[idx] = reading;
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
