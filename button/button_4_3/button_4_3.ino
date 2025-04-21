#include <Key.h>
#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns

// byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
// byte colPins[COLS] = {6, 8, 7}; //connect to the column pinouts of the keypad
byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 8, 7}; //connect to the column pinouts of the keypad

// char keys[ROWS][COLS] = {
//   {'1','2','3','A'},
//   {'4','5','6','B'},
//   {'7','8','9','C'},
//   {'#','0','*','D'}
// };

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'#','0','*'}
};

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup() {
  Serial.begin(9600);
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    Serial.println(key);
  }
}