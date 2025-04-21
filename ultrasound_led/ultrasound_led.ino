#define TRIG_PIN 9
#define ECHO_PIN 10

#define RED_LED 6
#define GREEN_LED 7

long duration;
float distance;

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

  Serial.print("거리: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance < 10.0) {  // 10cm 이내에 물체가 있으면 "사용중"
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    Serial.println("상태: 사용중");
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    Serial.println("상태: 사용가능");
  }

  delay(500);  // 0.5초마다 측정
}