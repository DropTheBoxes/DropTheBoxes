int red = 13;
int green = 12;

void setup() {
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) { // 시리얼 데이터가 수신된 경우
    char receivedChar = Serial.read(); // 시리얼로부터 데이터 읽기
    if (receivedChar == '1') {
      digitalWrite(red, HIGH); // 받은 데이터가 '1'이면 LED 켜기
      digitalWrite(green, LOW);
    } else if (receivedChar == '0') {
      digitalWrite(green, HIGH); // 받은 데이터가 '0'이면 LED 끄기
      digitalWrite(red, LOW);
    }
  }
}
