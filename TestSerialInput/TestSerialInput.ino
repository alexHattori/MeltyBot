void setup() {
 Serial.begin(9600);

}
int incomingByte = 0;   // for incoming serial data

void loop() {
  if (Serial.available() > 0) {
                // read the incoming byte:
                incomingByte = Serial.read();

                // say what you got:
                Serial.print("I received: ");
                Serial.println(incomingByte, DEC);
        }
}
