//자석여부를 검출하는 실습입니다.
 
int dig2 = 2; // 센서핀 D2 설정
 
int mag = 1; // 센서값 저장변수 설정
 
void setup() { //초기화
 
Serial.begin(9600); //통신속도 설정
 
pinMode(dig2, INPUT); // 센서핀 입력설정
 
}
 
void loop() { //무한루프
 
mag = digitalRead(dig2); //디지털값을 읽어와 저장
 
Serial.print(" Magnetic : ");
 
if(mag == 0 ) { Serial.println(" Y "); } //자석이 감지되면 Y 출력
 
else { Serial.println(" N "); } //감지가 안되면 N출력
 
delay(1000); 
 
}
