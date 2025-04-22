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
  {'*','0','#'}
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

// 입력된 숫자 자릿수 카운터 (0-4)
int digitCount = 0;

// 디스플레이 업데이트 필요 여부
bool displayNeedsUpdate = true;

// 디버그 모드 설정
const bool DEBUG = true;

// 라즈베리 파이 통신 설정
const char START_MARKER = '<';
const char END_MARKER = '>';

void setup() {
  Serial.begin(9600);
  
  // 모든 핀 초기화
  initializePins();
  
  // 초기 화면: "0000"
  setDisplayNumber("0000");
  
  // 시작 메시지 출력
  printStartupMessage();
}

void loop() {
  // 키패드 입력 처리
  char key = keypad.getKey();
  
  if (key) {
    processKeyInput(key);
  }
  
  // 디스플레이 업데이트가 필요한 경우에만 업데이트
  if (displayNeedsUpdate) {
    updateDisplay();
  }
}

// 핀 초기화 함수
void initializePins() {
  // 모든 디지트 핀 초기화
  for(int i=0; i<4; i++) {
    pinMode(DigitPins[i], OUTPUT);
    digitalWrite(DigitPins[i], DigitOff); // 모든 디지트 비활성화
  }
  
  // 모든 세그먼트 핀 초기화
  for(int i=0; i<8; i++) {
    pinMode(SegmentPins[i], OUTPUT);
    digitalWrite(SegmentPins[i], SegOff); // 모든 세그먼트 끄기
  }
}

// 시작 메시지 출력 함수
void printStartupMessage() {
  Serial.println("============================================");
  Serial.println("7세그먼트 4자리 표시기 + 키패드 입력 시스템");
  Serial.println("============================================");
  Serial.println("사용 방법:");
  Serial.println("- 숫자 키(0-9): 숫자 입력 (최대 4자리)");
  Serial.println("- * 키: 확인 (현재 값 전송 후 초기화)");
  Serial.println("- # 키: 백스페이스 (마지막 숫자 삭제)");
  Serial.println("============================================");
  Serial.println("현재 표시 값: 0000");
  Serial.println("============================================");
}

// 키패드 입력 처리
void processKeyInput(char key) {
  printKeyInputInfo(key);
  
  bool displayChanged = false;
  
  if(key >= '0' && key <= '9') {
    // 숫자 키: 4자리가 모두 입력되지 않은 경우에만 처리
    if(digitCount < 4) {
      // 숫자 추가 (오른쪽에서 왼쪽으로 이동)
      shiftNumbersLeft();
      displayNumber[3] = key;
      digitCount++;
      displayChanged = true;
      
      Serial.print("숫자 입력: ");
      Serial.println(key);
    } else {
      Serial.println("이미 4자리가 모두 입력되었습니다. 더 이상 입력할 수 없습니다.");
    }
  } else if(key == '*') {
    // * 키: 확인 (현재 값 전송 후 초기화)
    sendCurrentValue();
    resetDisplay();
    displayChanged = true;
  } else if(key == '#') {
    // # 키: 백스페이스 (마지막 숫자 삭제)
    if(digitCount > 0) {
      // 마지막 숫자 삭제 (오른쪽에서 왼쪽으로)
      shiftNumbersRight();
      displayNumber[0] = '0';
      digitCount--;
      displayChanged = true;
      Serial.println("백스페이스 실행: 마지막 숫자 삭제");
    } else {
      Serial.println("삭제할 숫자가 없습니다.");
    }
  }
  
  // 디스플레이가 변경된 경우에만 업데이트
  if(displayChanged) {
    setDisplayNumber(displayNumber);
    // 디스플레이 업데이트 필요 표시
    displayNeedsUpdate = true;
  }
  
  // 현재 표시 값 출력
  printCurrentValue();
}

// 키 입력 정보 출력 함수
void printKeyInputInfo(char key) {
  Serial.println("--------------------------------------------");
  Serial.print("키 입력: ");
  Serial.println(key);
}

// 현재 표시 값 출력 함수
void printCurrentValue() {
  Serial.print("현재 표시 값: ");
  Serial.println(displayNumber);
  
  // 숫자 값으로 변환하여 출력
  int numericValue = atoi(displayNumber);
  Serial.print("숫자 값: ");
  Serial.println(numericValue);
  Serial.print("입력된 자릿수: ");
  Serial.println(digitCount);
  Serial.println("--------------------------------------------");
}

// 현재 값 전송
void sendCurrentValue() {
  int numericValue = atoi(displayNumber);
  
  Serial.println(">> 확인 버튼 누름: 값 전송");
  
  // 라즈베리 파이로 데이터 전송
  sendDataToRaspberryPi(numericValue);
  
  Serial.println(">> 디스플레이 초기화");
}

// 라즈베리 파이로 데이터 전송 함수
void sendDataToRaspberryPi(int value) {
  // 시작 마커 전송
  Serial.print(START_MARKER);
  
  // 데이터 전송
  Serial.print(value);
  
  // 종료 마커 전송
  Serial.println(END_MARKER);
  
  // 디버그 정보 출력
  if(DEBUG) {
    Serial.print("라즈베리 파이로 전송된 데이터: ");
    Serial.print(START_MARKER);
    Serial.print(value);
    Serial.print(END_MARKER);
    Serial.println();
  }
}

// 디버그 정보 출력
void printDebugInfo() {
  Serial.println("현재 디스플레이 상태:");
  for(int digit = 0; digit < 4; digit++) {
    Serial.print("Digit ");
    Serial.print(digit);
    Serial.print(": ");
    for(int seg = 0; seg < 8; seg++) {
      Serial.print(displayData[digit][seg]);
    }
    Serial.println();
  }
  Serial.println();
}

// 숫자를 왼쪽으로 시프트
void shiftNumbersLeft() {
  for(int i = 0; i < 3; i++) {
    displayNumber[i] = displayNumber[i + 1];
  }
}

// 숫자를 오른쪽으로 시프트
void shiftNumbersRight() {
  for(int i = 3; i > 0; i--) {
    displayNumber[i] = displayNumber[i - 1];
  }
}

// 디스플레이 리셋 (0000)
void resetDisplay() {
  clearDisplayNumber();
  digitCount = 0;
}

// 디스플레이 숫자 초기화 (모두 0으로)
void clearDisplayNumber() {
  for(int i = 0; i < 4; i++) {
    displayNumber[i] = '0';
  }
  displayNumber[4] = '\0';
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
