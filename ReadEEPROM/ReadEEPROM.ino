#include <EEPROM.h>

int eepAddr1 = 1;
int eepAddr2 = 2;
void setup() {
  Serial.begin(9600);

}

void loop() {
  byte val = EEPROM.read(eepAddr1);
  byte val2 = EEPROM.read(eepAddr2);

  Serial.print((int)val*4);
  Serial.print(" ");
  Serial.println((int)val2)*5;
  
}
