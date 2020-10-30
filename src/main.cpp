#include <Arduino.h>
#include <BleKeyboard.h>

BleKeyboard bleKeyboard("Mute O Matic", "ERK", 100);

// Button states
int buttonState = 0;
int lastButtonState = 0;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Pins
const int LED_PIN = 13;
const int BUTTON_PIN = 39;

// Meet
const char z = 0x7A;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  bleKeyboard.begin();

  while(!bleKeyboard.isConnected()) {
    Serial.print(".");
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
  }
  Serial.println();
  Serial.println("Keyboard connected");
}

void toggle() {
  digitalWrite(LED_PIN, HIGH);

  bleKeyboard.press(KEY_LEFT_ALT);
  bleKeyboard.press(z);
  delay(20);
  bleKeyboard.releaseAll();
  delay(20);

  digitalWrite(LED_PIN, LOW);
}


void checkButton() {
  int reading = digitalRead(BUTTON_PIN);

  // Has it changed? Due to noise or pressing
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // Has it been in this state long enough?
  if ((millis() - lastDebounceTime) > debounceDelay) {

    // Has the signal changed?
    if (reading != buttonState) {
      buttonState = reading;

      // Is it a press
      if (reading == false) {
        toggle();
        Serial.println("Button pressed");
      }
    }
  }
  lastButtonState = reading;
}

void loop() {
  if(bleKeyboard.isConnected()) {
    checkButton();
  }
}
