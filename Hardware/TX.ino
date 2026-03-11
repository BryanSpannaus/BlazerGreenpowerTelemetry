#include <SPI.h>
#include <RH_RF95.h> // Version 1.121
#include <cmath>
#include <cstdint>
#include <vector>
#include <SD.h>
#include <TC74.h>

//Radio Pins
#define RF95_CS   17
#define RF95_INT  20
#define RF95_RST  21

#define RF95_FREQ 915.0

//EEPROM Addresses

#define EEPROM_0PAGE_0 0x50
#define EEPROM_0PAGE_1 0x51

#define EEPROM_1PAGE_0 0x50
#define EEPROM_1PAGE_1 0x51

#define EEPROM_2PAGE_0 0x50
#define EEPROM_2PAGE_1 0x51

#define EEPROM_3PAGE_0 0x50
#define EEPROM_3PAGE_1 0x51

//Input Pins
#define RPM_SENSE 1
#define VOLTAGE_DIVIDER A0

//Output Pins
#define BUZZER 0
#define SDcs 7

//Lat and Long transmit points
#define TRANSMISSION_LONG
#define TRANSMISSION_LAT

struct DATA_BUFFER {
  float RPM; //RPMs
  float MOTOR_TEMP; //Motor
  float AMBIENT_TEMP; //From MCU
  float AMP_LOAD; 
  float VOLTAGE; //Batt voltage
  float X_ACEL; //Accelerometer data
  float Y_ACEL;
};

void transmit(float param1, float param2, float param3, float param4, struct DATA_BUFFER *foo);
void saveData(struct DATA_BUFFER *foo);
void EEPROM_SAVE(struct DATA_BUFFER *foo);

//RPM Calculating Variables (used in hallISR, and Loop)
volatile unsigned long lastPulseTime = 0;
volatile unsigned long pulseInterval = 0;
volatile bool newPulse = false;

TC74 dvc(0x48); //Initalizes TC74A03.3VAT as a sensor

RH_RF95 rf95(RF95_CS, RF95_INT);
//GPT Code
void hallISR() {
  unsigned long now = micros();
  if (lastPulseTime != 0) {
    pulseInterval = now - lastPulseTime;
    newPulse = true;
  }
  lastPulseTime = now;
}
//GPT Code End

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RF95_CS, OUTPUT);
  pinMode(SDcs, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(VOLTAGE_DIVIDER, INPUT);
  pinMode(RPM_SENSE, INPUT_PULLUP);
  pinMode(RF95_RST, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(RF95_RST, HIGH);
  SPI.begin(); 
  SD.begin(SDcs);
  attachInterrupt(digitalPinToInterrupt(RPM_SENSE), hallISR, FALLING);
  Serial.begin(115200);
  dvc.begin(); //Start Temparature Sensor
  // Reset LoRa module
  digitalWrite(SDcs, HIGH);

  digitalWrite(RF95_CS, LOW);
  digitalWrite(RF95_RST, LOW);
  delay(10);
  digitalWrite(RF95_RST, HIGH);
  delay(10);
  digitalWrite(RF95_CS, HIGH);

  if (!rf95.init()) { //init fail
    Serial.println("LoRa radio init failed");
    while (1);
  }
  
  Serial.println("LoRa radio init OK!");  
  if (!rf95.setFrequency(RF95_FREQ)) { 
    Serial.println("setFrequency failed");
    while (1);
  }

  Serial.print("Set Freq to: ");
  Serial.println(RF95_FREQ);

  digitalWrite(SDcs, LOW);
  rf95.setTxPower(20, false);
  if (SD.exists("/telem")) {
  } else {
  SD.mkdir("telem");
  }
/* //Depricated use EEPROM instead
  File dataLog = SD.open("TELEM.csv", FILE_WRITE);
  dataLog.println("RPM, M_TMP, A_TMP, AMPS, VOLTS, LAT, LONG, X_ACEL, Y_ACEL, UTC_HR, UTC_MIN, UTC_SEC, LAP, LAPT_TIME, TIME, LORA_SNR"); //Generate Header for file
  dataLog.close(); //Write changes to file
  digitalWrite(SDcs, HIGH);
*/
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  //Declare Variables
  float AMPS;
  int lastUpdateTime;
  float battVoltage;
  float temp;

  if (digitalRead(RPM_SENSE) == LOW) {
    digitalWrite(BUZZER, HIGH);
  } else {
    digitalWrite(BUZZER, LOW);
  }

  struct DATA_BUFFER TEMP_BUF;

  //unsigned long lastCalcTime = 0;
  float rpm = 0.0;
  //GPT Code
  if (newPulse) {
    noInterrupts();
    unsigned long interval = pulseInterval;
    newPulse = false;
    interrupts();

    if (interval > 0) {
      rpm = (60.0 * 1000000.0) / interval; // 1 pulse per rev
    }
  }
  //GPT Code End
  TEMP_BUF.RPM = rpm;

  battVoltage = (analogRead(VOLTAGE_DIVIDER) * 24.8)/1005; //I have no idea where this proportion came from, testing
  TEMP_BUF.VOLTAGE = battVoltage;

  temp = dvc.readTemperature('c');
  TEMP_BUF.MOTOR_TEMP = temp;

  transmit(AMPS, battVoltage, rpm, temp, &TEMP_BUF); 
}

void transmit(float param1, float param2, float param3, float param4, struct DATA_BUFFER *foo) {
  digitalWrite(RF95_CS, LOW); //Enable RF95 (Pull SPI CS Pin LOW)
  char dataSendSTR[25]; //Init 25 byte string with 4 bytes padding (assuming 1 byte per char)
  sprintf(dataSendSTR, "%f.2, %f.2, %f.1, %f.1,\0", param1, param2, param3, param4); //Format data
  Serial.println(dataSendSTR);
  rf95.send((uint8_t *)dataSendSTR, sizeof(dataSendSTR)); //Send to LoRa buffers
  rf95.waitPacketSent(); 
  digitalWrite(RF95_CS, HIGH); //Disable RF95 (Push SPI CS Pin HIGH)
 // saveData(foo); //Save to SD Card
 loop();
}

void saveData(struct DATA_BUFFER *foo) {
  digitalWrite(SDcs, LOW); //Enable SD Card (Pull SPI CS Pin LOW)
  int time = floor(millis()/1000);
  File dataLog = SD.open("TELEM.csv", FILE_WRITE);
  dataLog.printf("%f.1, %f.1, %f.1, %f.2, %f.2, %f.3, %f.3, %i", foo->RPM, foo->MOTOR_TEMP, foo->AMBIENT_TEMP, foo->AMP_LOAD, foo->VOLTAGE, foo->X_ACEL, foo->Y_ACEL, time);
  dataLog.close();
  digitalWrite(SDcs, HIGH); //Disable SD Card (Push SPI CS Pin HIGH)
  loop();
}

void setup1(){
  //Setup buffers for data to save with this core
}

void loop1(){
  //This core will do TX/EEPROM, Core 0 needs to just gather data and send it
}

//Implement this function in loop1
void EEPROM_SAVE(struct DATA_BUFFER *foo) {
//Format it first 
  float rpm_E; //6 Chars
  if (foo->RPM > 9999.9) {
    rpm_E = 9999.9;
  } else {
    rpm_E = foo->RPM;
  }
  
  float temp_M_E; //5 Chars
  if (foo->MOTOR_TEMP > 999.9) {
    temp_M_E = 999.9;
  } else { 
    temp_M_E = foo->MOTOR_TEMP;
  }

  float temp_AMB_E; //5 Chars
  if (foo->AMBIENT_TEMP > 999.9) {
    temp_AMB_E = 999.9;
  } else {
    temp_AMB_E = foo->AMBIENT_TEMP;
  }

  float AMPS_E; //5 Chars
  if (foo->AMP_LOAD > 99.99) {
    AMPS_E = 99.99;
  } else {
    AMPS_E = foo->AMP_LOAD;
  }

  float VOLTAGE_E; //5 Chars
  if (foo->VOLTAGE > 99.99) {
    VOLTAGE_E = 99.99;
  } else {
    VOLTAGE_E = foo->VOLTAGE;
  }

  float X_ACEL_E; //7 chars
  if (foo->X_ACEL > 999.999) {
    X_ACEL_E = 999.999;
  } else {
    X_ACEL_E = foo->X_ACEL;
  }
  
  float Y_ACEL_E; //7 chars
  if (foo->Y_ACEL > 999.999) {
    Y_ACEL_E = 999.999;
  } else {
    Y_ACEL_E = foo->Y_ACEL;
  }
  
//Find seconds elapsed 
int time_E = floor(millis()/1000); //4 chars
  if (time_E > 9999) {
    time_E = 9999;
  }

char EEPROM_DATA[53]; //44 Chars without commas, 52 with commas, 53 with null termination
sprintf(EEPROM_DATA, "%f.1,%f.1,%f.1,%f.2,%f.2,%f.3,%f.3,%i,\0", rpm_E, temp_M_E, temp_AMB_E, AMPS_E, VOLTAGE_E, X_ACEL_E, Y_ACEL_E, time_E); //Format data

  //now the hard part, write through I2C to the right addr, right chip, etc.
  
}
