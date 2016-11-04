void setup() {
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  digitalWrite(7,LOW);
}

void loop() {
  digitalWrite(8,HIGH);
  digitalWrite(8,LOW);

}
