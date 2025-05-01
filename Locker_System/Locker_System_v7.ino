#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>

// 핀 설정
// RFID
#define SS_PIN 53 // SDA
#define RST_PIN 49
MFRC522 rfid(SS_PIN, RST_PIN);

// 초음파
#define TRIG_PIN 23
#define ECHO_PIN 25

// 홀센서
#define HALL_PIN 29

// 솔레노이드 제어 릴레이
#define LOCK 39 // 잠금용
#define PUSH 41 // 문 push용

// LED
#define RED 33
#define GREEN 35

#define BUZZER_PIN 12

// 4digit 7-segment
// 핀 설정
const int DigitPins[4] = {34, 40, 42, 32}; // Digit 제어 핀 (LED 핀 6, 8, 9, 12)
const int SegmentPins[8] = {36, 44, 28, 24, 22, 38, 30, 26}; // a, b, c, d, e, f, g, dp 

// 키패드 설정
const byte ROWS = 4; // 4개 행
const byte COLS = 4; // 3개 열
char keys[ROWS][COLS] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

// keypad
const byte rowPins[ROWS] = {2, 3, 4, 5}; // 키패드 행 핀
const byte colPins[COLS] = {6, 7, 8, 9};     // 키패드 열 핀
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


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

const bool NOAC[4][8] = {
  {0, 0, 1, 0, 1, 0, 1, 0}, // n (c, e, g 세그먼트)
  {0, 0, 1, 1, 1, 0, 1, 0}, // o (c, d, e, g 세그먼트)
  {1, 1, 1, 0, 1, 1, 1, 0}, // A (a, b, c, e, f, g 세그먼트)
  {0, 0, 0, 1, 1, 0, 1, 0}  // c (d, e, g 세그먼트)
};

// 표시할 값 저장용 배열
bool displayData[4][8] = {0};  // 4자리 x 8세그먼트

// 현재 표시 중인 숫자 (문자열)
char displayNumber[5] = "0000";

// 4자리 비밀번호 + null 종료 문자
char secretCode[5] = "";
String receivedPassword = "";

// 입력된 숫자 자릿수 카운터 (0-4)
int digitCount = 0;


// 디스플레이 업데이트 필요 여부
bool displayNeedsUpdate = true;


// 전역 변수
bool authorized = false;
bool objectDetected = false;
unsigned long hallSensorStartTime = 0;
String currentStatus = "";
String newStatus = "";

// 전역 변수 추가
unsigned long lastStatusChangeTime = 0;
const unsigned long debounceDelay = 500; // 500ms

static bool solenoidActive = true;

static int count=0;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  Serial.println("RFID 보관함 시스템 시작");
  // 모든 핀 초기화
  initializePins();

  // 초기 화면: "0000"
  setDisplayNumber("0000");
  updateDisplay(); // 초기 디스플레이 업데이트 직접 호출
  // 시작 메시지 출력
  // printStartupMessage();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(HALL_PIN, INPUT);

  pinMode(LOCK, OUTPUT);
  digitalWrite(LOCK, LOW);
  pinMode(PUSH, OUTPUT);
  digitalWrite(PUSH, LOW);

  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);
}

void loop() {
  static unsigned long lastSensorCheck = 0;
  unsigned long currentMillis = millis();

  static String lastStatus = "";
  
  // 디스플레이 업데이트는 매 루프마다 수행
  updateDisplay();
  
  // 센서 측정은 100ms마다 수행
  if (currentMillis - lastSensorCheck >= 100) {
    lastSensorCheck = currentMillis;
    processSerialInput();
    objectDetected = detectObject();
    sendStatus();
    LEDStatus();
    
    if(newStatus == "사용가능") {
      checkRFID();
      keyProcess();
    } else if(newStatus == "사용중") {
      keyProcess();
    }

    checkHallSensor();
  }
}

void processSerialInput() {
  if (Serial.available() > 0) {
    String str = "";
    while (Serial.available() > 0) {
      char c = Serial.read();
      if ((c >= '0' && c <= '9')) {
        str += c;
      }else if(c == 'T' || c == 'S'){
        str += c;
      }else if(!(c >= '0' && c <= '9') && (c == 'O' || c == 'P' || c == 'E' || c == 'N')){
        str += c;
      }
      delay(2);
    }
    str.trim();
    Serial.println(str);
    // 입력이 4자리 숫자일 때만 비밀번호로 저장
    if (str.length() == 4) {
      str.toCharArray(secretCode, 5); // 4글자 + null 종료
      Serial.print("비밀번호가 설정되었습니다: ");
      Serial.println(secretCode);
    } else if (str.length() > 0 && str!="T" && str!="S") {
      Serial.println("비밀번호는 4자리 숫자여야 합니다.");
    }

    if (str.length() == 1 && authorized) {  
      char command = str.charAt(0);
      if (command == 'T') {
        Serial.println("A 입력 감지: 솔레노이드 전원 차단!");
        solenoidActive = false;
        solenoidLow();
      } else if (command == 'S') {
        Serial.println("B 입력 감지: 솔레노이드 전원 재활성화!");
        solenoidActive = true;
        solenoidHigh2();
      }
    }

    if(str == "OPEN"){
      Serial.println("웹 명령: 솔레노이드 동작!");
      authorized = true;
      startPassAndSolenoid();
      setDisplayNumber("0000");
      updateDisplay();
    }
  }
}




void keyProcess() {
  char key = keypad.getKey();
  
  if (key) {
    processKeyInput(key);
  }
  
  // 디스플레이 업데이트는 loop()에서 처리하므로 여기서는 제거
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
  bool detect = detectObject();
  bool displayChanged = false;

  if(!authorized) {
    if(newStatus=="사용가능"){
      displayNoAc();
      displayChanged = true;
      return; // 다른 키 처리는 하지 않고 종료
    }
  } else{
    if(key == 'A'){
      solenoidActive = !solenoidActive;
      Serial.println(solenoidActive);
      if(solenoidActive){
        Serial.println("A 키 입력: 솔레노이드 활성화");
        solenoidHigh2();
      }else{
        Serial.println("A 키 입력: 솔레노이드 차단");
        solenoidLow();
      }
      return;
    }
  }
  if(!authorized && detect){
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

void displayNoAc() {
  clearDisplay();
    
  // NOAC 패턴 복사
  for(int digit = 0; digit < 4; digit++) {
      for(int seg = 0; seg < 8; seg++) {
          displayData[digit][seg] = NOAC[digit][seg];
       }
  }
    
  displayNeedsUpdate = true;
  
  // PASS 표시를 유지하면서 솔레노이드 작동 시간 동안 대기
  unsigned long startTime = millis();
  while(millis() - startTime < 3000) {
    updateDisplay();
  }
  clearDisplay();
  resetDisplay();
  setDisplayNumber("0000");
  displayNeedsUpdate = true; // 디스플레이 업데이트 필요 표시
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

void updateDisplay() {
  static int currentDigit = 0;  // 현재 활성화할 디지트 인덱스
  
  // 모든 디지트 비활성화
  for(int i = 0; i < 4; i++) {
    digitalWrite(DigitPins[i], DigitOff);
  }
  
  // 모든 세그먼트 끄기
  for(int seg = 0; seg < 8; seg++) {
    digitalWrite(SegmentPins[seg], SegOff);
  }
  
  // 현재 디지트의 세그먼트 설정
  for(int seg = 0; seg < 8; seg++) {
    digitalWrite(SegmentPins[seg], displayData[currentDigit][seg] ? SegOn : SegOff);
  }
  
  // 현재 디지트 활성화
  digitalWrite(DigitPins[currentDigit], DigitOn);
  
  // 다음 디지트로 이동
  currentDigit = (currentDigit + 1) % 4;
  
  // 짧은 지연 (멀티플렉싱 타이밍)
  delayMicroseconds(500);  // delay(5)를 delayMicroseconds(500)로 변경
}

void sendPassword(){
  Serial.print("비밀번호: ");
  Serial.println(displayNumber);

  comparePassword();
}

void comparePassword(){
  if(strcmp(displayNumber, secretCode)==0){
    Serial.println("비밀번호 일치");
    Serial.println("PASSWORD 보관함 열림");
    count=0;
    // PASS 표시와 솔레노이드 동작을 동시에 처리
    authorized = true;
    startPassAndSolenoid();
  } else {
      Serial.println("비밀번호 불일치");
      failCount();
      startFailDisplay();
  }
}

void startPassAndSolenoid() {
  clearDisplay();

  // PASS 패턴 복사
  for(int digit = 0; digit < 4; digit++) {
    for(int seg = 0; seg < 8; seg++) {
      displayData[digit][seg] = PASS[digit][seg];
    }
  }
  
  // 솔레노이드 활성화
  solenoidHigh1();
  
  // 타이머 설정 (3초 후 디스플레이 초기화)
  unsigned long displayTimer = millis();
  while(millis() - displayTimer < 3000) {
    updateDisplay();
    delay(1);  // 다른 작업을 위한 시간 제공
  }
  Serial.println("OPEN");
  strcpy(secretCode, "");
  // 디스플레이 초기화
  clearDisplay();
}

void startFailDisplay(){
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
  while(millis() - startTime < 1000) { // 3초 동안 표시 및 솔레노이드 작동
    updateDisplay();
  }

  // 디스플레이 초기화
  clearDisplay();
}

// RFID 인증 함수
void checkRFID() {
  if (authorized) {
    // 이미 인증된 상태면 RFID 다시 읽지 않음
    return;
  }

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String uidStr = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      uidStr += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
      uidStr += String(rfid.uid.uidByte[i], HEX);
    }
    uidStr.toUpperCase();
    Serial.print("RFID UID: "); Serial.println(uidStr);

    if (uidStr == "86C1DE1F") {
      Serial.println("RFID 보관함 열림");
      solenoidHigh1();
      authorized = true;
      delay(1000);
    } else {
      Serial.println("인증 실패");
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}

bool detectObject() {
  // 이전 상태 저장
  String previousStatus = newStatus;
  
  // 초음파 센서 측정
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // 타임아웃 설정 (최대 20ms)
  long duration = pulseIn(ECHO_PIN, HIGH, 20000);
  
  // 타임아웃 발생 시 처리
  if (duration == 0) {
    return false;
  }
  
  float distance = duration * 0.034 / 2;
  newStatus = (distance < 10.0) ? "사용중" : "사용가능";

  if(previousStatus!=newStatus){
    Serial.println("상태 변화");
    clearDisplay();
    resetDisplay();
    setDisplayNumber("0000");
    updateDisplay();
  }

  return distance > 0 && distance < 10;
}

// 홀센서(배송기사) 감지 함수
void checkHallSensor() {
  int mag = digitalRead(HALL_PIN);

  if (mag == LOW) {  // 감지 시작
    if (hallSensorStartTime == 0) {
      hallSensorStartTime = millis();
    } else if (millis() - hallSensorStartTime >= 3000) {
      Serial.println("3초 이상 홀센서 감지 → 보관함 잠금");
      // Serial.println("상태: 사용중");
      digitalWrite(LOCK, LOW);
      digitalWrite(PUSH, LOW);
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
    Serial.println("상태: "+newStatus);
  }
}

void recivePassword(){
  if(Serial.available() > 0){
    while(Serial.available() > 0){
      char c = Serial.read();
      receivedPassword += c;
      delay(2);
    }
  }
    receivedPassword.trim();
}

void solenoidHigh1() {
  digitalWrite(PUSH, HIGH);
  delay(500);
  digitalWrite(LOCK, HIGH);
  delay(500);
  digitalWrite(PUSH, LOW);
  delay(500);
  digitalWrite(PUSH, HIGH);
}

void solenoidHigh2() {
  digitalWrite(PUSH, HIGH);
  digitalWrite(LOCK, HIGH);
}

void solenoidLow(){
  digitalWrite(PUSH, LOW);
  digitalWrite(LOCK, LOW);
}

void failCount(){
  count++;

  if(count>=5){
    Serial.println("재발급");
    count=0;
    buzzer(1500,500);
  }
}

void buzzer(int frequency, int duration) {
  tone(BUZZER_PIN, frequency);
  delay(duration);
  noTone(BUZZER_PIN);
}