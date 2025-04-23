#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define TRIG_PIN 3
#define ECHO_PIN 2
#define HALL_PIN 5
#define RELAY_PIN 4

MFRC522 rfid(SS_PIN, RST_PIN);

bool authorized = false; // RFID 인증 상태
bool objectDetected = false; // 물체 감지 상태
unsigned long hallSensorStartTime = 0; // 홀센서 감지 시간
String currentStatus = "";  // 보관함 상태 "사용중" 또는 "사용가능"

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("RFID 보관함 시스템 시작");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(HALL_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // 시작 시 잠금 상태
}

void loop() {
  // 1. RFID 인증
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String uidStr = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      uidStr += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
      uidStr += String(rfid.uid.uidByte[i], HEX);
    }
    uidStr.toUpperCase();
    Serial.print("RFID UID: "); Serial.println(uidStr);

    // 인증 UID 확인 (예: 86C1DE1F)
    if (uidStr == "86C1DE1F") {
      Serial.println("인증 완료! 보관함 열림");
      digitalWrite(RELAY_PIN, HIGH);  // 솔레노이드 열림
      authorized = true;
      delay(1000); // 너무 빠른 루프 방지
    } else {
      Serial.println("인증 실패");
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

// 2. 초음파 감지
if (authorized && !objectDetected && detectObject()) {
  Serial.println("물건 감지");
  objectDetected = true;
}

  // 3. 홀센서 감지 3초 이상
  if (authorized && objectDetected) {
    int mag = digitalRead(HALL_PIN); // LOW면 감지됨

    if (mag == LOW) {
      if (hallSensorStartTime == 0) {
        hallSensorStartTime = millis();
      } else if (millis() - hallSensorStartTime >= 3000) {
        Serial.println("3초 이상 홀센서 감지 → 보관함 잠금");
        Serial.println("상태: 사용중");
        digitalWrite(RELAY_PIN, LOW); // 솔레노이드 잠금
        authorized = false;
        objectDetected = false;
        hallSensorStartTime = 0;
        delay(1000);
      }
    } else {
      hallSensorStartTime = 0; // 감지 중단되면 타이머 리셋
    }
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

  // 10cm 이내면 감지
  return distance > 0 && distance < 10;
}