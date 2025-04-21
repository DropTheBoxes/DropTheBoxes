int ECHO_pin=3;
int TRIG_pin=2;
unsigned long distance;

void setup() {
  Serial.begin(9600);
  pinMode(ECHO_pin,INPUT);
  pinMode(TRIG_pin, OUTPUT);
}

void loop() {
  digitalWrite(TRIG_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_pin, LOW);
  
  distance=pulseIn(ECHO_pin, HIGH);
  distance=distance/58;
  Serial.println("---distance---");
  Serial.println("Detect an Obstacle in range[cm] of");
  Serial.println(String(distance) + " cm");
  Serial.println("--------------");
  delay(1000);
}
