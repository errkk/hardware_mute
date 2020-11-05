#include <Arduino.h>
#include <BleKeyboard.h>
#include <FastLED.h>

// WS2812 config
#define NUM_LEDS 25
#define DATA_PIN 27

// Define the array of leds
CRGB leds[NUM_LEDS];
const int BRIGHTNESS = 32;

// Instanciate bluetooth HID
BleKeyboard bleKeyboard("Mute O Matic", "ERK", 100);

// Button states
#define debounce 20 // ms debounce period to prevent flickering when pressing or releasing the button
#define holdTime 1000 // ms hold period: how long to wait for press+hold event

int buttonVal = 0; // value read from button
int buttonLast = 0; // buffered value of the button's previous state
long btnDnTime; // time the button was pressed down
long btnUpTime; // time the button was released
boolean ignoreUp = false; // whether to ignore the button release because the click+hold was triggered

//
int buttonState = 0;
int lastButtonState = 0;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
unsigned long pressHoldDelay = 2000;

// 0 Meet, 1 Teams, 2 Zoom
#define MEET 0
#define TEAMS 1
#define ZOOM 2

int mode = MEET;
CRGB modes[3] {CRGB::Orange, CRGB::Purple, CRGB::Blue};

// Blinking indicator state
unsigned long lastBlink = 0;
unsigned long blinkDelay = 800;
int ledState = 0;
CRGB ledColour = CRGB::Red;

// Pins
const int BUTTON_PIN = 39;

// Meet Alt Z
const char z = 0x7A;

// Teams Ctrl Shift M
const char m = 0x6D;

// Zoom
const char a = 0x61;

// Function defs
void doBlinking();
void nextMode();
void checkButton();
void displayMode();

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  pinMode(BUTTON_PIN, INPUT);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  FastLED.setBrightness(BRIGHTNESS);

  bleKeyboard.begin();

  while(!bleKeyboard.isConnected()) {
    Serial.print(".");
    ledColour = CRGB::Red;
    doBlinking();
  }
  Serial.println();
  Serial.println("Keyboard connected");
  displayMode();
}

void toggle() {
  switch (mode) {
  case MEET:
    // Meet
    bleKeyboard.press(KEY_LEFT_ALT);
    bleKeyboard.press(z);
    break;
  case TEAMS:
    // Teams
    bleKeyboard.press(KEY_LEFT_GUI);
    bleKeyboard.press(KEY_LEFT_SHIFT);
    bleKeyboard.press(m);
    break;
  case ZOOM:
    // Zoom
    bleKeyboard.press(KEY_LEFT_GUI);
    bleKeyboard.press(KEY_LEFT_SHIFT);
    bleKeyboard.press(a);
    break;
  }

  leds[4] = CRGB::Red;
  FastLED.show();

  delay(20);
  bleKeyboard.releaseAll();
  leds[4] = CRGB::Black;
  FastLED.show();
}

void checkButton() {
  // Read the state of the button
  buttonVal = digitalRead(BUTTON_PIN);

  // Test for button pressed and store the down time
  if (buttonVal == LOW && buttonLast == HIGH && (millis() - btnUpTime) > long(debounce)) {
    btnDnTime = millis();
  }

  // Test for button release and store the up time
  if (buttonVal == HIGH && buttonLast == LOW && (millis() - btnDnTime) > long(debounce)) {
    if (ignoreUp == false) {
      toggle();
    } else {
      ignoreUp = false;
    }
    btnUpTime = millis();
  }

  // Test for button held down for longer than the hold time
  if (buttonVal == LOW && (millis() - btnDnTime) > long(holdTime)) {
    nextMode();
    ignoreUp = true;
    btnDnTime = millis();
  }

  buttonLast = buttonVal;
}

void nextMode() {
  if (mode < ZOOM) {
    mode ++;
  } else {
    mode = MEET;
  }
  displayMode();
}

void displayMode() {
  // Turn off leds 1,2,3
  leds[MEET + 1] = CRGB::Black;
  leds[TEAMS + 1] = CRGB::Black;
  leds[ZOOM + 1] = CRGB::Black;
  leds[mode + 1] = modes[mode];
  FastLED.show();
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

