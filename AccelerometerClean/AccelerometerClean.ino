void setup() {
  pinMode(11,OUTPUT);
  pinMode(12,OUTPUT);
  digitalWrite(11,HIGH);
  digitalWrite(12,LOW);
  Serial.begin(9600);
}
void loop() {
  int val = analogRead(A0);
  Serial.println(val);

}
