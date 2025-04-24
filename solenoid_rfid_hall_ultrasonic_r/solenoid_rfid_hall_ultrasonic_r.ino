#include <SPI.h>
#include <MFRC522.h>

// 핀 설정
#define SS_PIN 10
#define RST_PIN 9
#define TRIG_PIN 3
#define ECHO_PIN 2
#define HALL_PIN 5
#define RELAY_PIN 4

MFRC522 rfid(SS_PIN, RST_PIN);

// 전역 변수
bool authorized = false;
bool objectDetected = false;
unsigned long hallSensorStartTime = 0;
String currentStatus = "";

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("RFID 보관함 시스템 시작");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(HALL_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
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

void loop() {
  // 1. RFID 확인
  checkRFID();

  // 2. 초음파 감지 (사용중 판정)
  if (authorized && !objectDetected && detectObject()) {
    Serial.println("물건 감지");
    objectDetected = true;
  }

  // 3. 홀센서 체크
  if (authorized && objectDetected) {
    checkHallSensor();
  }
}
