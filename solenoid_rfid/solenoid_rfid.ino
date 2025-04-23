#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);

const int relay = 4;

// 상태 유지 변수
bool isAuthorized = false;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  Serial.println(F("RFID 시작"));

  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);  // 기본적으로 잠금 상태
}

void loop() {
  if ( !rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

  String content = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    content.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(rfid.uid.uidByte[i], HEX));
  }
  content.toUpperCase();

  Serial.print("RFID UID: ");
  Serial.println(content);

  // 인증된 UID 검사
  if (content.substring(1) == "86 C1 DE 1F") {
    Serial.println("인증 완료: 보관함 열림");
    digitalWrite(relay, HIGH);
    isAuthorized = true;
  } else {
    Serial.println("인증 실패: 보관함 잠금");
    digitalWrite(relay, LOW);
    isAuthorized = false;
  }

  // 리더기 상태 초기화
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(1000);  // 과도한 태그 인식 방지
}