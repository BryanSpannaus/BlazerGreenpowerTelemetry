#include "EXT_EEPROM.h"

EEPROM::EEPROM(uint8_t addr) {
    _addr = addr;
}

void write(uint32_t memaddr, byte data, uint8_t dvcaddr){
    Wire.beginTransmission(dvcaddr);
    Wire.write((int)highByte(memaddr));
    Wire.write((int)lowByte(memaddr));
    Wire.write(data);
    Wire.endTransmission();
}

byte read(uint32_t memaddr, uint8_t dvcaddr){
    byte data = '\0';
    Wire.beginTransmission(dvcaddr);
    Wire.write((int)highByte(memaddr));
    Wire.write((int)lowByte(memaddr));
    Wire.endTransmission();
    Wire.requestFrom(dvcaddr, 1);
    data = Wire.read();
    return data;
}
