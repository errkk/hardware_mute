#include <Arduino.h>
#include <BleKeyboard.h>

BleKeyboard bleKeyboard("Mute O Matic", "ERK", 100);

#define UNMUTE 0
#define MUTE 1

#define SWITCH 0
#define BUTTON 1

// Button states
int buttonState[2] = {0, 0};
int lastButtonState[2] = {0, 0};

unsigned long lastDebounceTime[2] = {0, 0};
unsigned long debounceDelay = 50;
String label[2] = {"Unmute", "Mute"};

// Pins
const int BATT_PIN = A13;
const int LED_PIN = 13;
const int LED_EX_PIN = 15;
const int SWITCH_PIN = 27;
const int BUTTON_PIN = 33;

// Battery
const float MAX_ADC = 4095;
const float REF_VOLTAGE = 3.3;
const float ADC_REF_VOLTAGE = 1.1;
unsigned long lastBatteryCheck = 0;
unsigned long batteryInterval = 30000;
uint8_t batteryPercentage;

// Teams
const char m = 0x6D;
const char z = 0x7A;

const int PINS[2] = {SWITCH_PIN, BUTTON_PIN};
const int KEYS[2] = {0x97, 0x98};


void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_EX_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(SWITCH_PIN, INPUT);

  bleKeyboard.begin();

  while(!bleKeyboard.isConnected()) {
    Serial.print(".");
    digitalWrite(LED_EX_PIN, HIGH);
    delay(100);
    digitalWrite(LED_EX_PIN, LOW);
  }
  Serial.println();
  Serial.println("Keyboard connected");
}

void sendKeys(int idx) {
  digitalWrite(LED_EX_PIN, HIGH);

  Serial.println(label[idx]);
  bleKeyboard.press(KEY_LEFT_ALT);
  bleKeyboard.press(KEYS[idx]);
  delay(50);
  bleKeyboard.releaseAll();
  delay(50);

  digitalWrite(LED_EX_PIN, LOW);
}

void toggle() {
  digitalWrite(LED_EX_PIN, HIGH);

  bleKeyboard.press(KEY_LEFT_GUI);
  bleKeyboard.press(KEY_LEFT_CTRL);
  bleKeyboard.press(m);
  delay(50);
  bleKeyboard.releaseAll();
  bleKeyboard.press(KEY_LEFT_ALT);
  bleKeyboard.press(z);
  delay(50);
  bleKeyboard.releaseAll();
  delay(50);

  digitalWrite(LED_EX_PIN, LOW);
}


void checkButton() {
  int idx = BUTTON;
  int reading = digitalRead(PINS[idx]);

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
      if (reading == false) {
        toggle();
        Serial.println("Button pressed");
      }
    }
  }
  lastButtonState[idx] = reading;
}

void checkSwitch() {
  int idx = SWITCH;

  int reading = digitalRead(PINS[idx]);

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
      if (reading == false) {
        sendKeys(UNMUTE);
        Serial.println("Switch on");
      } else {
        sendKeys(MUTE);
        Serial.println("Switch off");
      }
    }
  }
  lastButtonState[idx] = reading;
}

void checkBattery() {
  int reading = analogRead(BATT_PIN);
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
    checkButton();
    checkSwitch();
    checkBattery();
  }
}
