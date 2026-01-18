#include <FastLED.h>

// Pins for Components
#define RED_BTN 1
#define GREEN_BTN 2
#define BUZZER 18
#define RGB_PIN 48
#define NUM_LEDS 4

// 7-Segment Pins (a-g)
const int segPins[] = {4, 5, 6, 7, 15, 16, 17};
const byte numbers[8] = {
  0b00111111, 0b00000110, 0b01011011, 0b01001111, 
  0b01100110, 0b01101101, 0b01111101, 0b00000111
};

// Feedback LEDs (Yellow replaces Red)
int yellowLEDs[] = {8, 9, 10, 11};
int greenLEDs[] = {12, 13, 14, 21};

CRGB leds[NUM_LEDS];
int secretCode[4];
int userGuess[4] = {0, 0, 0, 0};
int currentPos = 0;
int tries = 7;

CRGB palette[] = {CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::Yellow, CRGB::Purple, CRGB::Cyan};

void setup() {
  pinMode(RED_BTN, INPUT_PULLUP);
  pinMode(GREEN_BTN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  randomSeed(analogRead(0)); // Improve randomness
  
  for(int i=0; i<7; i++) pinMode(segPins[i], OUTPUT);
  for(int i=0; i<4; i++) {
    pinMode(yellowLEDs[i], OUTPUT);
    pinMode(greenLEDs[i], OUTPUT);
  }

  FastLED.addLeds<WS2812B, RGB_PIN, GRB>(leds, NUM_LEDS);
  resetGame();
}

void loop() {
  if (digitalRead(RED_BTN) == LOW && digitalRead(GREEN_BTN) == LOW) {
    resetGame();
    delay(500); 
  }

  if (digitalRead(RED_BTN) == LOW) {
    userGuess[currentPos] = (userGuess[currentPos] + 1) % 6;
    leds[currentPos] = palette[userGuess[currentPos]];
    FastLED.show();
    tone(BUZZER, 440, 1000); 
    delay(300); 
  }

  if (digitalRead(GREEN_BTN) == LOW) {
    tone(BUZZER, 660, 100); delay(150);
    tone(BUZZER, 660, 100);
    currentPos++;
    
    if (currentPos >= 4) {
      checkGuess();
      currentPos = 0;
    }
    delay(300);
  }
}

void checkGuess() {
  int perfectMatch = 0; // Green
  int colorMatch = 0;   // Yellow
  
  bool secretUsed[4] = {false, false, false, false};
  bool guessUsed[4] = {false, false, false, false};

  // First Pass: Find Correct Color + Correct Position (Green)
  for (int i = 0; i < 4; i++) {
    if (userGuess[i] == secretCode[i]) {
      perfectMatch++;
      secretUsed[i] = true;
      guessUsed[i] = true;
    }
  }

  // Second Pass: Find Correct Color + Wrong Position (Yellow)
  for (int i = 0; i < 4; i++) {
    if (guessUsed[i]) continue; 
    for (int j = 0; j < 4; j++) {
      if (!secretUsed[j] && userGuess[i] == secretCode[j]) {
        colorMatch++;
        secretUsed[j] = true;
        break; 
      }
    }
  }

  updateFeedbackLEDs(perfectMatch, colorMatch);

  if (perfectMatch == 4) {
    winMelody();
    resetGame();
  } else {
    tries--;
    displayNumber(tries);
    if (tries <= 0) {
      flatline();
      resetGame();
    } else {
      anxietyBeeps();
    }
  }
}

void updateFeedbackLEDs(int greenCount, int yellowCount) {
  // Clear all first
  for(int i=0; i<4; i++) {
    digitalWrite(greenLEDs[i], LOW);
    digitalWrite(yellowLEDs[i], LOW);
  }
  // Light up Green LEDs (1st to greenCount-th LED)
  for(int i=0; i < greenCount; i++) {
    digitalWrite(greenLEDs[i], HIGH);
  }
  // Light up Yellow LEDs (starting after the Green ones)
  for(int i=0; i < yellowCount; i++) {
    digitalWrite(yellowLEDs[i], HIGH);
  }
}

void displayNumber(int n) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segPins[i], bitRead(numbers[n], i));
  }
}

void resetGame() {
  tries = 7;
  currentPos = 0;
  displayNumber(tries);
  for (int i = 0; i < 4; i++) {
    secretCode[i] = random(0, 6);
    userGuess[i] = 0;
    leds[i] = CRGB::Black;
    digitalWrite(yellowLEDs[i], LOW);
    digitalWrite(greenLEDs[i], LOW);
  }
  FastLED.show();
}

void winMelody() {
  int mel[] = {523, 659, 784, 1047};
  for(int i=0; i<4; i++) { tone(BUZZER, mel[i], 150); delay(200); }
}

void anxietyBeeps() {
  for(int i=0; i<3; i++) { tone(BUZZER, 300, 50); delay(100); }
}

void flatline() {
  tone(BUZZER, 150, 3000);
  delay(3000);
}
