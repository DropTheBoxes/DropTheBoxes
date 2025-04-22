// 7세그먼트 4자리 표시기 (오른쪽 정렬 버전)
// by Perplexity

// 핀 설정
const int DigitPins[4]   = {2, 3, 4, 5};        // Digit 제어 핀 (D1~D4)
const int SegmentPins[8] = {6, 7, 8, 9, 10, 11, 12, 13}; // a~g, dp

// 논리값 정의 (공통 애노드 기준)
const bool DigitOn  = LOW;
const bool DigitOff = HIGH;
const bool SegOn    = HIGH;
const bool SegOff   = LOW;

// 숫자 배열 (dp 제외)
const int BLANK[8] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
const int N0[8]    = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW, LOW};
const int N1[8]    = {LOW, HIGH, HIGH, LOW, LOW, LOW, LOW, LOW};
const int N2[8]    = {HIGH, HIGH, LOW, HIGH, HIGH, LOW, HIGH, LOW};
const int N3[8]    = {HIGH, HIGH, HIGH, HIGH, LOW, LOW, HIGH, LOW};
const int N4[8]    = {LOW, HIGH, HIGH, LOW, LOW, HIGH, HIGH, LOW};
const int N5[8]    = {HIGH, LOW, HIGH, HIGH, LOW, HIGH, HIGH, LOW};
const int N6[8]    = {HIGH, LOW, HIGH, HIGH, HIGH, HIGH, HIGH, LOW};
const int N7[8]    = {HIGH, HIGH, HIGH, LOW, LOW, LOW, LOW, LOW};
const int N8[8]    = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW};
const int N9[8]    = {HIGH, HIGH, HIGH, HIGH, LOW, HIGH, HIGH, LOW};
const int MIN[8]   = {LOW, LOW, LOW, LOW, LOW, LOW, HIGH, LOW};

// 표시할 값 저장용 배열
const int* lights[4];

// 시리얼 입력 버퍼
char incoming[9] = {};

void setup() {
  Serial.begin(9600);

  // 핀모드 설정
  for (byte i = 0; i < 4; i++) pinMode(DigitPins[i], OUTPUT);
  for (byte i = 0; i < 8; i++) pinMode(SegmentPins[i], OUTPUT);

  // 초기값: "1234"
  lights[0] = N1;
  lights[1] = N2;
  lights[2] = N3;
  lights[3] = N4;
}

void loop() {
  if (Serial.available() > 0) {
    readSerialInput();
    parseInput();
    printDebug();
  }
  displaySegments();
}

// 시리얼 데이터 읽기 (최대 8자리, 소수점 필터링)
void readSerialInput() {
  memset(incoming, 0, sizeof(incoming));
  int i = 0;
  while (Serial.available() > 0 && i < sizeof(incoming)-1) {
    char c = Serial.read();
    if (c != '.' && c != ',') incoming[i++] = c;
    delay(3);
  }
  incoming[i] = '\0'; // 문자열 종료
}

// 입력값 파싱 및 오른쪽 정렬
void parseInput() {
  // 모든 자리 초기화
  for (int i=0; i<4; i++) lights[i] = BLANK;

  // 유효 입력 길이 계산
  int len = strnlen(incoming, 8);
  len = (len > 4) ? 4 : len; // 최대 4자리
  
  // 오른쪽 정렬 시작 위치
  int startPos = 4 - len;

  // 자릿수 매핑
  for (int i=0; i<len; i++) {
    int pos = startPos + i;
    if(pos >= 4) break;
    
    switch(incoming[i]) {
      case '0': lights[pos] = N0; break;
      case '1': lights[pos] = N1; break;
      case '2': lights[pos] = N2; break;
      case '3': lights[pos] = N3; break;
      case '4': lights[pos] = N4; break;
      case '5': lights[pos] = N5; break;
      case '6': lights[pos] = N6; break;
      case '7': lights[pos] = N7; break;
      case '8': lights[pos] = N8; break;
      case '9': lights[pos] = N9; break;
      case '-': lights[pos] = MIN; break;
      default:  lights[pos] = BLANK; break;
    }
  }
}

// 7세그먼트 표시
void displaySegments() {
  for (byte seg=0; seg<8; seg++) {
    digitalWrite(SegmentPins[seg], SegOn);
    for (byte digit=0; digit<4; digit++) {
      digitalWrite(DigitPins[digit], (lights[digit][seg] == 1) ? DigitOn : DigitOff);
    }
    delayMicroseconds(2500); // 밝기 조절
    digitalWrite(SegmentPins[seg], SegOff);
    for (byte digit=0; digit<4; digit++) digitalWrite(DigitPins[digit], DigitOff);
  }
}

// 디버그 출력
void printDebug() {
  Serial.print("Input: ");
  Serial.println(incoming);
  for (int y=0; y<4; y++) {
    Serial.print("Digit ");
    Serial.print(y);
    Serial.print(": ");
    for (int z=0; z<8; z++) Serial.print(lights[y][z]);
    Serial.println();
  }
}
