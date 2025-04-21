int echoPin = 2;
int trigPin = 3;

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  digitalWrite(trigPin, LOW);
  digitalWrite(echoPin, LOW);
  delay(300);

  digitalWrite(trigPin, HIGH);
  delay(1000);
  digitalWrite(trigPin, LOW);
  
  unsigned long time = pulseIn (echoPin, HIGH)
  float distance = ((float)(340*time)/10000)/2; // 소리는 1초에 340m * time = 왕복거리 

  Serial.print(distance);
  Serial.println("cm");

}

