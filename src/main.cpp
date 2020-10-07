#include <Arduino.h>
#include <Keyboard.h>

#define UNMUTE 0
#define MUTE 1

#define SWITCH 0
#define BUTTON 1

// Button states
int buttonState;
int lastButtonState;

unsigned long lastDebounceTime;
unsigned long debounceDelay = 50;

// Pins
const int LED_PIN = PB4;
const int BUTTON_PIN = PB3;
// SDA - PB0
// SCL - PB2

//       __
// PB5 -|  |- VCC
// PB3 -|  |- PB2
// PB4 -|  |- PB1
// GND -|__|- PB0

// GMeet
const char z = 0x7A;

const int PINS[1] = {BUTTON_PIN};
const int KEYS[1] = {0x98};


void setup() {
  //Serial.begin(9600);
  //Serial.println("Starting...");

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  Keyboard.begin();
}


void toggle() {
  digitalWrite(LED_PIN, HIGH);

  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.press(z);
  delay(50);
  Keyboard.releaseAll();
  delay(50);

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
        //Serial.println("Button pressed");
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
