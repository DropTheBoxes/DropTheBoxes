#include <Keypad.h>
#define GREEN 12
#define RED 11

String receivedPassword = "";

// 키패드 설정
const byte ROWS = 4; // 4개 행
const byte COLS = 3; // 3개 열
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

char displayNumber[5] = "0000";
int digitCount = 0;
byte rowPins[ROWS] = {2, 3, 4, 5}; // 키패드 행 핀
byte colPins[COLS] = {6, 7, 8};     // 키패드 열 핀
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


// 디버그 모드 설정
const bool DEBUG = true;

void setup() {
  Serial.begin(9600);

  // 모든 핀 초기화
  initializePins();

  // 초기 화면: "0000"
  setDisplayNumber("0000");
  
  // 시작 메시지 출력
  printStartupMessage();

  Serial.println("비밀번호 수신 대기 중...");

  pinMode(GREEN, OUTPUT);
  digitalWrite(GREEN, LOW);
  pinMode(RED, OUTPUT);
  digitalWrite(RED, LOW);
}

//----------------------loop-------------------------------
void loop() {
  if(Serial.available() > 0){
    while(Serial.available() > 0){
      char c = Serial.read();
      receivedPassword += c;
      delay(2);
    }
  }
  receivedPassword.trim();
  // 키패드 입력 처리
  keyProcess(); 
}

void keyProcess(){
   char key = keypad.getKey();
  
  if (key) {
    processKeyInput(key);
  }
  
}

// 핀 초기화 함수
void initializePins() {
  // 모든 디지트 핀 초기화
  for(int i=0; i<4; i++) {
    // pinMode(DigitPins[i], OUTPUT);
    // digitalWrite(DigitPins[i], DigitOff); // 모든 디지트 비활성화
  }
  
  // 모든 세그먼트 핀 초기화
  for(int i=0; i<8; i++) {
    // pinMode(SegmentPins[i], OUTPUT);
    // digitalWrite(SegmentPins[i], SegOff); // 모든 세그먼트 끄기
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
}

// 키 입력 정보 출력 함수
void printKeyInputInfo(char key) {
  Serial.print("키 입력: ");
  Serial.println(key);
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
    comparePassword();
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
    // displayNeedsUpdate = true;
  }
  
  // 현재 표시 값 출력
  printCurrentValue();
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
      // displayData[digit][seg] = 0;
    }
  }
}

// 특정 자리에 숫자 설정
void setDigitNumber(int digit, int number) {
  // 범위 검사
  if(digit < 0 || digit > 3 || number < 0 || number > 9) return;
  
  // 숫자 패턴 복사
  for(int seg = 0; seg < 8; seg++) {
    // displayData[digit][seg] = NUMBERS[number][seg];
  }
}

// 디스플레이 업데이트 (멀티플렉싱)
void updateDisplay() {
  // 각 디지트 순차적으로 활성화
  for(int digit = 0; digit < 4; digit++) {
    // 모든 디지트 비활성화
    for(int i = 0; i < 4; i++) {
      // digitalWrite(DigitPins[i], DigitOff);
    }
    
    // 모든 세그먼트 끄기 (고스팅 방지)
    for(int seg = 0; seg < 8; seg++) {
      // digitalWrite(SegmentPins[seg], SegOff);
    }
    
    // 현재 디지트의 세그먼트 설정
    for(int seg = 0; seg < 8; seg++) {
      // digitalWrite(SegmentPins[seg], displayData[digit][seg] ? SegOn : SegOff);
    }
    
    // 현재 디지트 활성화
    // digitalWrite(DigitPins[digit], DigitOn);
    
    // 짧은 지연 (멀티플렉싱 타이밍)
    delay(2);
  }
}

void sendPassword(){
  Serial.print("라즈베리 파이로 비밀번호 전송: ");
  Serial.print("비밀번호: ");
  // Serial.println(displayNumber);

  comparePassword();
}

void comparePassword(){
  String p="";
  for(int i=0;i<4;i++){
    p+=displayNumber[i];
  }
  Serial.println("p: "+p);
  if(p == receivedPassword){
    if(DEBUG){
      Serial.println("비밀번호 일치");
    }
    // PASS 표시와 솔레노이드 동작을 동시에 처리
    
    digitalWrite(GREEN, HIGH);
    digitalWrite(RED, LOW);
  } else {
    if(DEBUG){
      Serial.println("비밀번호 불일치");
    }
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
  }
}
