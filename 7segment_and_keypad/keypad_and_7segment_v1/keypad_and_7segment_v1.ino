#include <Keypad.h>

// 7세그먼트 4자리 표시기 (4자리 숫자만 표시하는 버전)

// 핀 설정
const int DigitPins[4] = {2, 3, 4, 5};        // Digit 제어 핀 (D1~D4)
const int SegmentPins[8] = {6, 7, 8, 9, 10, 11, 12, 13}; // a~g, dp

// 키패드 설정
const byte ROWS = 4; // 4개 행
const byte COLS = 3; // 3개 열
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'#','0','*'}
};
byte rowPins[ROWS] = {22, 24, 26, 28}; // 키패드 행 핀
byte colPins[COLS] = {30, 32, 34};     // 키패드 열 핀
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// 논리값 정의 (공통 애노드 기준)
const bool DigitOn  = LOW;
const bool DigitOff = HIGH;
const bool SegOn    = HIGH;
const bool SegOff   = LOW;

// 숫자 배열 (a, b, c, d, e, f, g, dp)
const bool NUMBERS[10][8] = {
  {1, 1, 1, 1, 1, 1, 0, 0},    // 0
  {0, 1, 1, 0, 0, 0, 0, 0},    // 1
  {1, 1, 0, 1, 1, 0, 1, 0},    // 2
  {1, 1, 1, 1, 0, 0, 1, 0},    // 3
  {0, 1, 1, 0, 0, 1, 1, 0},    // 4
  {1, 0, 1, 1, 0, 1, 1, 0},    // 5
  {1, 0, 1, 1, 1, 1, 1, 0},    // 6
  {1, 1, 1, 0, 0, 0, 0, 0},    // 7
  {1, 1, 1, 1, 1, 1, 1, 0},    // 8
  {1, 1, 1, 1, 0, 1, 1, 0}     // 9
};

// 표시할 값 저장용 배열
bool displayData[4][8] = {0};  // 4자리 x 8세그먼트

// 현재 표시 중인 숫자 (문자열)
char displayNumber[5] = "0000";

// 디버그 모드 설정
const bool DEBUG = true;

void setup() {
  Serial.begin(9600);
  
  // 모든 핀 초기화
  for(int i=0; i<4; i++) {
    pinMode(DigitPins[i], OUTPUT);
    digitalWrite(DigitPins[i], DigitOff); // 모든 디지트 비활성화
  }
  
  for(int i=0; i<8; i++) {
    pinMode(SegmentPins[i], OUTPUT);
    digitalWrite(SegmentPins[i], SegOff); // 모든 세그먼트 끄기
  }
  
  // 초기 화면: "0000"
  setDisplayNumber("0000");
  
  // 시작 메시지 출력
  Serial.println("============================================");
  Serial.println("7세그먼트 4자리 표시기 + 키패드 입력 시스템");
  Serial.println("============================================");
  Serial.println("사용 방법:");
  Serial.println("- 숫자 키(0-9): 숫자 입력");
  Serial.println("- # 키: 확인 (현재 값 전송 후 초기화)");
  Serial.println("- * 키: 백스페이스 (마지막 숫자 삭제)");
  Serial.println("============================================");
  Serial.println("현재 표시 값: 0000");
  Serial.println("============================================");
}

void loop() {
  // 키패드 입력 처리
  char key = keypad.getKey();
  
  if (key) {
    processKeyInput(key);
  }
  
  // 디스플레이 업데이트
  updateDisplay();
}

// 키패드 입력 처리
void processKeyInput(char key) {
  Serial.println("--------------------------------------------");
  Serial.print("키 입력: ");
  Serial.println(key);
  
  if(key >= '0' && key <= '9') {
    // 숫자 키: 왼쪽으로 시프트하고 새 숫자 추가
    shiftNumbersLeft();
    displayNumber[3] = key;
    Serial.print("숫자 입력: ");
    Serial.println(key);
  } else if(key == '#') {
    // # 키: 확인 (현재 값 전송 후 초기화)
    sendCurrentValue();
    resetDisplay();
  } else if(key == '*') {
    // * 키: 백스페이스 (마지막 숫자 삭제)
    backspace();
    Serial.println("백스페이스 실행: 마지막 숫자 삭제");
  }
  
  // 디스플레이 업데이트
  setDisplayNumber(displayNumber);
  
  // 현재 표시 값 출력
  Serial.print("현재 표시 값: ");
  Serial.println(displayNumber);
  
  // 숫자 값으로 변환하여 출력
  int numericValue = atoi(displayNumber);
  Serial.print("숫자 값: ");
  Serial.println(numericValue);
  Serial.println("--------------------------------------------");
}

// 현재 값 전송
void sendCurrentValue() {
  int numericValue = atoi(displayNumber);
  
  Serial.println(">> 확인 버튼 누름: 값 전송");
  Serial.print(">> 전송된 값: ");
  Serial.println(numericValue);
  
  // 여기에 값 전송 관련 코드 추가 가능
  // 예: 다른 장치로 값 전송, 데이터 처리 등
  
  Serial.println(">> 디스플레이 초기화");
}

// 숫자를 왼쪽으로 시프트
void shiftNumbersLeft() {
  for(int i = 0; i < 3; i++) {
    displayNumber[i] = displayNumber[i + 1];
  }
}

// 디스플레이 리셋 (0000)
void resetDisplay() {
  strcpy(displayNumber, "0000");
}

// 백스페이스 (마지막 숫자 삭제하고 오른쪽에 0 추가)
void backspace() {
  shiftNumbersRight();
  displayNumber[0] = '0';
}

// 숫자를 오른쪽으로 시프트
void shiftNumbersRight() {
  for(int i = 3; i > 0; i--) {
    displayNumber[i] = displayNumber[i - 1];
  }
}

// 숫자 문자열을 디스플레이에 설정
void setDisplayNumber(const char* numStr) {
  // 모든 디스플레이 데이터 초기화
  clearDisplay();
  
  // 각 자리 숫자 설정
  for(int i = 0; i < 4; i++) {
    char c = numStr[i];
    
    if(c >= '0' && c <= '9') {
      // 숫자 0-9
      setDigitNumber(i, c - '0');
    } else {
      // 인식할 수 없는 문자는 0으로 표시
      setDigitNumber(i, 0);
    }
  }
}

// 디스플레이 데이터 초기화
void clearDisplay() {
  for(int digit = 0; digit < 4; digit++) {
    for(int seg = 0; seg < 8; seg++) {
      displayData[digit][seg] = 0;
    }
  }
}

// 특정 자리에 숫자 설정
void setDigitNumber(int digit, int number) {
  // 범위 검사
  if(digit < 0 || digit > 3 || number < 0 || number > 9) return;
  
  // 숫자 패턴 복사
  for(int seg = 0; seg < 8; seg++) {
    displayData[digit][seg] = NUMBERS[number][seg];
  }
}

// 디스플레이 업데이트 (멀티플렉싱)
void updateDisplay() {
  // 각 디지트 순차적으로 활성화
  for(int digit = 0; digit < 4; digit++) {
    // 모든 디지트 비활성화
    for(int i = 0; i < 4; i++) {
      digitalWrite(DigitPins[i], DigitOff);
    }
    
    // 모든 세그먼트 끄기 (고스팅 방지)
    for(int seg = 0; seg < 8; seg++) {
      digitalWrite(SegmentPins[seg], SegOff);
    }
    
    // 현재 디지트의 세그먼트 설정
    for(int seg = 0; seg < 8; seg++) {
      digitalWrite(SegmentPins[seg], displayData[digit][seg] ? SegOn : SegOff);
    }
    
    // 현재 디지트 활성화
    digitalWrite(DigitPins[digit], DigitOn);
    
    // 짧은 지연 (멀티플렉싱 타이밍)
    delay(2);
  }
}
