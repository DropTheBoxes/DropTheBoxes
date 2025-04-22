#define TRIG_PIN 11
#define ECHO_PIN 10

#define RED_LED 8
#define GREEN_LED 9

long duration;
float distance;
String currentStatus = "";  // 현재 상태 저장 변수

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  // 초음파 신호 보내기
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Echo에서 거리 측정
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;  // cm 단위

  String newStatus = (distance < 10.0) ? "사용중" : "사용가능";

  // 상태가 바뀌었을 때만 시리얼 전송
  if (newStatus != currentStatus) {
    currentStatus = newStatus;

    // LED 표시
    if (newStatus == "사용중") {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
    } else {
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
    }

    Serial.println("상태: " + newStatus);
  }

  delay(500);  // 0.5초마다 측정
}
