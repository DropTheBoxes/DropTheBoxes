#include <Key.h>
#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 3;

byte rowPins[ROWS] = {7, 6, 5, 4};
byte colPins[COLS] = {8, 10, 9};

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'#','0','*'}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// 4자리 숫자 저장 버퍼 (초기값 "0000")
char inputBuffer[5] = "0000"; 

void setup() {
  Serial.begin(9600);
  Serial.println("Start:" + String(inputBuffer)); // 초기값 출력
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    if (isDigit(key)) { // 숫자 키만 처리
      // 왼쪽으로 한 자리씩 시프트 (첫 번째 문자 제거)
      for(int i=0; i<3; i++) {
        inputBuffer[i] = inputBuffer[i+1];
      }
      inputBuffer[3] = key; // 새 숫자를 마지막 자리에 추가
      Serial.println(inputBuffer); // 결과 출력
    }
    else if (key == '#') { // #을 누르면 초기화
      strcpy(inputBuffer, "0000");
      Serial.println("Reset: 0000");
    }
    else if (key == '*') { // *을 누르면 입력값 출력 후 초기화
      Serial.print("Entered: ");
      Serial.println(inputBuffer);
      strcpy(inputBuffer, "0000");
      Serial.println("Reset: 0000");
    }
  }
}
