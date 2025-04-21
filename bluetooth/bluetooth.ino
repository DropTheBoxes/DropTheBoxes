#include <SoftwareSerial.h>
int RX = 11;
int TX = 10;
String btInput = "";
SoftwareSerial bluetooth(TX, RX);

unsigned long lastReceivedTime = 0; // 마지막 문자 수신 시간
const unsigned long TIMEOUT = 100;  // 100ms 동안 추가 데이터 없으면 문자열 처리

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
  Serial.println("블루투스 초기화 완료. 연결 대기 중...");
}

void loop() {
  // 블루투스 데이터 수신
  while (bluetooth.available()) {
    char c = bluetooth.read();
    btInput += c;
    lastReceivedTime = millis(); // 마지막 수신 시간 업데이트
  }

  // Timeout 발생 시 문자열 출력
  if (btInput.length() > 0 && (millis() - lastReceivedTime) > TIMEOUT) {
    Serial.print("블루투스 수신: ");
    Serial.println(btInput);
    btInput = ""; // 문자열 초기화
  }
}
