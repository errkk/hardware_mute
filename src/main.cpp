#include <Arduino.h>
#include <BleKeyboard.h>
#include <FastLED.h>

// WS2812 config
#define NUM_LEDS 1
#define DATA_PIN 27

// Define the array of leds
CRGB leds[NUM_LEDS];
const int BRIGHTNESS = 32;

// Instanciate bluetooth HID
BleKeyboard bleKeyboard("Mute O Matic", "ERK", 100);

// Button states
int buttonState = 0;
int lastButtonState = 0;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Blinking indicator state
unsigned long lastBlink = 0;
unsigned long blinkDelay = 800;
int ledState = 0;
CRGB ledColour = CRGB::Red;

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

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  FastLED.setBrightness(BRIGHTNESS);

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

bool blinkDelayComplete(unsigned long currentMillis) {
  return ((currentMillis - lastBlink) > blinkDelay);
}

void doBlinking() {
  unsigned long currentMillis = millis();

  if (blinkDelayComplete(currentMillis)) {
    if (ledState == 0) {
      leds[0] = ledColour;
      ledState = 1;
    } else {
      leds[0] = CRGB::Black;
      ledState = 0;
    }
    FastLED.show();

    lastBlink = currentMillis;
  }
}

void loop() {
  if(bleKeyboard.isConnected()) {
    checkButton();
    ledColour = CRGB::Green;
  } else {
    ledColour = CRGB::Red;
  }

  doBlinking();
}

