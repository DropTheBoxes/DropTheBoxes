#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>

// 핀 설정
// RFID
#define SS_PIN 3 // SOA
#define RST_PIN 2

// 초음파
#define TRIG_PIN 23
#define ECHO_PIN 25

// 홀센서
#define HALL_PIN 29

// 솔레노이드 제어 릴레이
#define RELAY_PIN_1 39
#define RELAY_PIN_2 41

// LED
#define RED 39
#define GREEN 41

// 4digit 7-segment
const int DigitPins[4] = {22, 24, 26, 28}; // Digit 제어 핀 (D1~D4)
const int SegmentPins[8] = {30, 32, 34, 36, 38, 40, 42, 44}; // a~g, dp

// keypad
byte rowPins[ROWS] = {7, 8, 9, 10}; // 키패드 행 핀
byte colPins[COLS] = {11, 12, 13};     // 키패드 열 핀
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// 키패드 설정
const byte ROWS = 4; // 4개 행
const byte COLS = 3; // 3개 열
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

MFRC522 rfid(SS_PIN, RST_PIN);

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


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
