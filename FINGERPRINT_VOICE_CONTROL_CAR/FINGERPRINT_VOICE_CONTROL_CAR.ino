void setup() {
  // put your setup code here, to run once:
  pinMode(A2, INPUT);
  pinMode(A0,OUTPUT);
  Serial.begin(9600);

}

(1023/1023)*5;

void loop() {
  // put your main code here, to run repeatedly:
int potReading = analogRead(A2);
float converting = (potReading/1023) * 5

Serial.println(converting);

analogWrite(A0,potReading);
delay(1000);
}
