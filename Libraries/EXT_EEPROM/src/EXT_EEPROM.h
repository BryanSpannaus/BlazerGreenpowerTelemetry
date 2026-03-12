#ifndef EXT_EEPROM
#define EXT_EEPROM

#include "Arduino.h"
#include "Wire.h"

class EEPROM {
    public:
    EEPROM(uint8_t addr);

    void write(uint32_t memaddr, byte data, uint8_t dvcaddr);

    byte read(uint32_t memaddr, uint8_t dvcaddr);

    private:
    uint8_t _addr;
}

#endif