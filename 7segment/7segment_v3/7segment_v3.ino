// 7세그먼트 4자리 표시기 (4자리 숫자만 표시하는 버전)

// 핀 설정
const int DigitPins[4] = {2, 3, 4, 5};        // Digit 제어 핀 (D1~D4)
const int SegmentPins[8] = {6, 7, 8, 9, 10, 11, 12, 13}; // a~g, dp

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

// 시리얼 입력 버퍼
char inputBuffer[9] = {0};

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
  
  if(DEBUG) Serial.println("7세그먼트 4자리 표시기 초기화 완료");
}

void loop() {
  // 시리얼 입력 처리
  if(Serial.available() > 0) {
    readSerialInput();
    setDisplayFromInput();
    if(DEBUG) printDebugInfo();
  }
  
  // 디스플레이 업데이트
  updateDisplay();
}

// 시리얼 데이터 읽기 (소수점 필터링)
void readSerialInput() {
  memset(inputBuffer, 0, sizeof(inputBuffer));
  int index = 0;
  
  // 시리얼 데이터 읽기
  while(Serial.available() > 0 && index < sizeof(inputBuffer)-1) {
    char c = Serial.read();
    if(c != '.' && c != ',') { // 소수점 필터링
      inputBuffer[index++] = c;
    }
    delay(2); // 시리얼 버퍼 안정화
  }
  
  inputBuffer[index] = '\0'; // 문자열 종료
  
  if(DEBUG) {
    Serial.print("입력 받은 데이터: ");
    Serial.println(inputBuffer);
  }
}

// 입력값에 따라 디스플레이 설정
void setDisplayFromInput() {
  // 입력이 비어있으면 무시
  if(strlen(inputBuffer) == 0) return;
  
  // 숫자만 필터링하여 새 버퍼에 저장
  char numericBuffer[9] = {0};
  int numIndex = 0;
  
  // 입력에서 숫자만 추출
  for(int i = 0; i < strlen(inputBuffer) && numIndex < 4; i++) {
    if(inputBuffer[i] >= '0' && inputBuffer[i] <= '9') {
      numericBuffer[numIndex++] = inputBuffer[i];
    } else {
      if(DEBUG) {
        Serial.print("숫자가 아닌 문자 '");
        Serial.print(inputBuffer[i]);
        Serial.println("'는 무시됩니다.");
      }
    }
  }
  
  // 숫자가 4자리보다 적으면 앞을 0으로 채우기
  if(numIndex < 4) {
    if(DEBUG) Serial.println("4자리보다 적은 숫자입니다. 앞을 0으로 채웁니다.");
    
    // 숫자를 임시 버퍼로 복사
    char tempBuffer[5] = {0};
    strcpy(tempBuffer, numericBuffer);
    
    // 앞부분을 0으로 채우기
    memset(numericBuffer, '0', 4);
    
    // 원래 숫자 복사 (뒤에서부터)
    for(int i = 0; i < numIndex; i++) {
      numericBuffer[4 - numIndex + i] = tempBuffer[i];
    }
    
    numericBuffer[4] = '\0';
  } else if(numIndex > 4) {
    // 4자리 초과 시 처음 4자리만 사용
    numericBuffer[4] = '\0';
    if(DEBUG) Serial.println("4자리를 초과하는 숫자입니다. 처음 4자리만 표시합니다.");
  }
  
  // 숫자가 없으면 "0000" 표시
  if(numIndex == 0) {
    if(DEBUG) Serial.println("숫자가 없습니다. 기본값 '0000'을 표시합니다.");
    strcpy(numericBuffer, "0000");
  }
  
  setDisplayNumber(numericBuffer);
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
      // 인식할 수 없는 문자는 0으로 표시 (이 부분은 이제 실행되지 않음)
      setDigitNumber(i, 0);
      if(DEBUG) {
        Serial.print("인식할 수 없는 문자 '");
        Serial.print(c);
        Serial.print("'가 위치 ");
        Serial.print(i);
        Serial.println("에 입력되었습니다. 0으로 표시합니다.");
      }
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
