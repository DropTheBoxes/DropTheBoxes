#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>

String receivedPassword = "";

// 핀 설정
#define SS_PIN 3
#define RST_PIN 2
#define TRIG_PIN 25
#define ECHO_PIN 23
#define HALL_PIN 29
#define RELAY_PIN 39
#define RED 33
#define GREEN 35

MFRC522 rfid(SS_PIN, RST_PIN);

// 키패드 설정
const byte ROWS = 4; // 4개 행
const byte COLS = 3; // 3개 열
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

// 핀 설정
const int DigitPins[4] = {22, 24, 26, 28}; // Digit 제어 핀 (D1~D4)
const int SegmentPins[8] = {30, 32, 34, 36, 38, 40, 42, 44}; // a~g, dp

byte rowPins[ROWS] = {7, 8, 9, 10}; // 키패드 행 핀
byte colPins[COLS] = {11, 12, 13};     // 키패드 열 핀
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

const bool PASS[4][8] = {
  {1, 1, 0, 0, 1, 1, 1, 0},  // P
  {1, 1, 1, 0, 1, 1, 1, 0},  // A
  {1, 0, 1, 1, 0, 1, 1, 0},  // S
  {1, 0, 1, 1, 0, 1, 1, 0}   // S
};

const bool FAIL[4][8] = {
  {1, 0, 0, 0, 1, 1, 1, 0},  // F
  {1, 1, 1, 0, 1, 1, 1, 0},  // A
  {0, 0, 0, 0, 1, 1, 0, 0},  // I
  {0, 0, 0, 1, 1, 1, 0, 0}   // L
};


// 표시할 값 저장용 배열
bool displayData[4][8] = {0};  // 4자리 x 8세그먼트

// 현재 표시 중인 숫자 (문자열)
char displayNumber[5] = "0000";

// 4자리 비밀번호 + null 종료 문자
char secretCode[5] = "1234";

// 입력된 숫자 자릿수 카운터 (0-4)
int digitCount = 0;
const int solenoidPin = 40;

// 디스플레이 업데이트 필요 여부
bool displayNeedsUpdate = true;

// 디버그 모드 설정
const bool DEBUG = true;

// 전역 변수
bool authorized = false;
bool objectDetected = false;
unsigned long hallSensorStartTime = 0;
String currentStatus = "";
String newStatus = "";

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // 모든 핀 초기화
  initializePins();

  rfid.PCD_Init();
  Serial.println("RFID 보관함 시스템 시작");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(HALL_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);

  // 초기 화면: "0000"
  setDisplayNumber("0000");
  
  // 시작 메시지 출력
  printStartupMessage();

  pinMode(solenoidPin, OUTPUT);
}

void loop() {
  // 1. RFID 확인
  checkRFID();

  // 2. 초음파 감지 (사용중 판정)
  objectDetected = detectObject();  // detectObject 함수가 이미 거리 감지 결과를 반환합니다

  // 3. 상태 업데이트 및 LED 제어
  sendStatus();

  // 4. 홀센서 체크
  if (authorized && objectDetected) {
    checkHallSensor();
  }

  // 라즈베리파이에서 비밀번호 받아오기
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
    displayNeedsUpdate = true;
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

void sendPassword(){
  Serial.print("라즈베리 파이로 비밀번호 전송: ");
  Serial.print("비밀번호: ");
  Serial.println(displayNumber);

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
  displayPassAndActivateSolenoid();
  } else {
    if(DEBUG){
      Serial.println("비밀번호 불일치");
    }
    displayFail();
  }
}

// PASS 표시와 솔레노이드 동작을 동시에 처리하는 함수
void displayPassAndActivateSolenoid(){
  clearDisplay();

  // PASS 패턴 복사
  for(int digit=0; digit<4; digit++){
    for(int seg=0; seg<8;seg++){
      displayData[digit][seg]=PASS[digit][seg];
    }
  }

  displayNeedsUpdate = true;
  
  // 솔레노이드 활성화

  digitalWrite(solenoidPin, HIGH);
  
  // PASS 표시를 유지하면서 솔레노이드 작동 시간 동안 대기
  unsigned long startTime = millis();
  while(millis() - startTime < 3000) { // 3초 동안 표시 및 솔레노이드 작동
    updateDisplay();
  }
  
  // 솔레노이드 비활성화
  digitalWrite(solenoidPin, LOW);
  
  // 디스플레이 초기화
  clearDisplay();
}

void displayFail(){
  clearDisplay();

  // FAIL 패턴 복사
  for(int digit=0; digit<4; digit++){
    for(int seg=0; seg<8;seg++){
      displayData[digit][seg]=FAIL[digit][seg];
    }
  }

  displayNeedsUpdate = true;
  
  // FAIL 표시를 유지하면서 작동 시간 동안 대기
  unsigned long startTime = millis();
  while(millis() - startTime < 3000) { // 3초 동안 표시 및 솔레노이드 작동
    updateDisplay();
  }

  // 디스플레이 초기화
  clearDisplay();
}

// RFID 인증 함수
void checkRFID() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String uidStr = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      uidStr += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
      uidStr += String(rfid.uid.uidByte[i], HEX);
    }
    uidStr.toUpperCase();
    Serial.print("RFID UID: "); Serial.println(uidStr);

    if (uidStr == "86C1DE1F") {
      Serial.println("인증 완료! 보관함 열림");
      digitalWrite(RELAY_PIN, HIGH);
      authorized = true;
      delay(1000);
    } else {
      Serial.println("인증 실패");
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
  if (objectDetected) {
  Serial.println("이미 사용중인 보관함 입니다. 비밀번호를 입력해주세요.");
  return;
}
}

// 초음파 감지 함수
bool detectObject() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;

  newStatus = (distance < 10.0) ? "사용중" : "사용가능";

  return distance > 0 && distance < 10;
}

// 홀센서 감지 함수
void checkHallSensor() {
  int mag = digitalRead(HALL_PIN);

  if (mag == LOW) {  // 감지 시작
    if (hallSensorStartTime == 0) {
      hallSensorStartTime = millis();
    } else if (millis() - hallSensorStartTime >= 3000) {
      Serial.println("3초 이상 홀센서 감지 → 보관함 잠금");
      Serial.println("상태: 사용중");
      digitalWrite(RELAY_PIN, LOW);
      authorized = false;
      objectDetected = false;
      hallSensorStartTime = 0;
      delay(1000);
    }
  } else {
    hallSensorStartTime = 0; // 감지 중단 시 리셋
  }
}

void LEDStatus(){
  if (newStatus == "사용중") {
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
  } else {
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, HIGH);
  }
}

void sendStatus(){
  if (newStatus != currentStatus) {
    currentStatus = newStatus;
    LEDStatus();
    Serial.println("상태: "+newStatus);
  }
}

