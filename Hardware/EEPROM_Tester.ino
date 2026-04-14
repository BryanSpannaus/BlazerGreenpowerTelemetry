#include <Wire.h>

#define datalength = 12;

void setup() {
  // put your setup code here, to run once:
  pinMode(1, OUTPUT);
  digitalWrite(1, LOW);
  Wire.begin();
  Serial.begin(115200);
  delay(1000);
  digitalWrite(1, HIGH);

  while(!Serial()) { }
  
  byte binarydata[datalength];
  char characterdata[datalength] = "Hello, there";
  memcpy(binarydata, characterdata, datalength);

  Serial.println("starting");

  for(int i = 0; i < datalength; i++) {
  eepromWrite(i, binarydata[i], 0x50);
  }
  delay(500);
  Serial.println(eepromRead(1, 0x50));
}

void loop() {
  for (int i = 0; i < datalength; i++) {
  Serial.print(eepromRead(i, 0x50), HEX);
  }
  Serial.println();
  delay(5000);
}


void eepromWrite(unsigned long int addr, byte data, unsigned int EEPROMaddr) {
  Wire.beginTransmission(EEPROMaddr);
  Wire.write((int)highByte(addr));
  Wire.write((int)lowByte(addr));
  Wire.write(data);
  Wire.endTransmission();
}

byte eepromRead(unsigned long int addr, unsigned int EEPROMaddr) {
  byte data = '\0';
  Wire.beginTransmission(EEPROMaddr);
  Wire.write((int)highByte(addr));
  Wire.write((int)lowByte(addr));
  Wire.endTransmission();
  Wire.requestFrom(EEPROMaddr, 1);
  data = Wire.read();
  return data;
}
