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

//Input Pins
#define RPM_SENSE 1
#define VOLTAGE_DIVIDER A0

//Output Pins
#define BUZZER 0
#define SDcs 22

//Lat and Long transmit points
#define TRANSMISSION_LONG
#define TRANSMISSION_LAT


struct DATA_BUFFER {
  float SEND_RPM; //RPMs
  float MOTOR_TEMP; //Motor
  float AMBIENT_TEMP; //From MCU
  float AMP_LOAD; 
  float VOLTAGE; //Batt voltage
  float LAT;
  float LONG;
  float X_ACEL; //Accelerometer data
  float Y_ACEL;
  int UTC_HOUR; //UTC time from GPS
  int UTC_MINUTE;
  int UTC_SECOND;
  int LAP; //# of laps calculated by GPS
  int LAP_TIME;
  int TOTAL_TIME; 
  int LoRa_SNR; //SNR (Signal to Noise Ratio)
};

void transmit(float param1, float param2, float param3, float param4);
void saveData(struct DATA_BUFFER *foo);

//RPM Calculating Variables (used in hallISR, and Loop)
volatile unsigned long lastPulseTime = 0;
volatile unsigned long pulseInterval = 0;
volatile bool newPulse = false;


TC74 dvc(0x48); //Initalizes TC74A03.3VAT as a sensor

RH_RF95 rf95(RF95_CS, RF95_INT);

void hallISR() {
  unsigned long now = micros();
  if (lastPulseTime != 0) {
    pulseInterval = now - lastPulseTime;
    newPulse = true;
  }
  lastPulseTime = now;
}

void setup() {

  pinMode(BUZZER, OUTPUT);
  pinMode(VOLTAGE_DIVIDER, INPUT);
  pinMode(RPM_SENSE, INPUT_PULLUP);
  pinMode(RF95_RST, OUTPUT);
  digitalWrite(RF95_RST, HIGH);
  SPI.begin(); 
  SD.begin(SDcs);
  attachInterrupt(digitalPinToInterrupt(RPM_SENSE), hallISR, FALLING);
  Serial.begin(115200);
  dvc.begin(); //Start Temparature Sensor
  // Reset LoRa module
  digitalWrite(RF95_RST, LOW);
  delay(10);
  digitalWrite(RF95_RST, HIGH);
  delay(10);

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

  rf95.setTxPower(20, false);
  if (SD.exists("/telem")) {
  } else {
  SD.mkdir("telemDATA");
  }

  File dataLog = SD.open("TELEM.csv", FILE_WRITE);
  dataLog.println("RPM, M_TMP, A_TMP, AMPS, VOLTS, LAT, LONG, X_ACEL, Y_ACEL, UTC_HR, UTC_MIN, UTC_SEC, LAP, LAPT_TIME, TIME, LORA_SNR"); //Generate Header for file
  dataLog.close(); //Write changes to file

  digitalWrite(BUZZER, HIGH);
  delay(150);
  digitalWrite(BUZZER, LOW);
}

void loop() {
  float AMPS;
  int lastUpdateTime;
  float battVoltage;
  float temp;

  struct DATA_BUFFER TEMP_BUF;

  //unsigned long lastCalcTime = 0;
  float rpm = 0.0;

  if (newPulse) {
    noInterrupts();
    unsigned long interval = pulseInterval;
    newPulse = false;
    interrupts();

    if (interval > 0) {
      rpm = (60.0 * 1000000.0) / interval; // 1 pulse per rev
    }
  }
  TEMP_BUF.SEND_RPM = rpm;

  battVoltage = (analogRead(VOLTAGE_DIVIDER) * 24.8)/1005;
  TEMP_BUF.VOLTAGE = battVoltage;

  if (millis() >= lastUpdateTime + 1000) {
    transmit(AMPS, battVoltage, rpm, temp);
    lastUpdateTime = millis(); //Reset counter
  }

  temp = dvc.readTemperature('c');
  TEMP_BUF.MOTOR_TEMP = temp;

  saveData(&TEMP_BUF);
}

void transmit(float param1, float param2, float param3, float param4) {
  float data[4];
  char dataSendSTR[25]; //Init 25 byte string with 4 bytes padding (assuming 1 byte per char)
  data[0] = param1;
  data[1] = param2;
  data[2] = param3;
  data[3] = param4;
  sprintf(dataSendSTR, "%f.2, %f.2, %f.1, %f.1,\0", data[1], data[2], data[3], data[4]); //Format data
  rf95.send((uint8_t *)dataSendSTR, sizeof(dataSendSTR)); //Send to LoRa buffers
  rf95.waitPacketSent();
}

void saveData(struct DATA_BUFFER *foo) {
  File dataLog = SD.open("TELEM.csv", FILE_WRITE);
  dataLog.printf("%f.1, %f.1, %f.1, %f.2, %f.2, %f.6, %f.6, %f.3, %f.3, %i, %i, %i, %i, %i, %i, %i", foo->SEND_RPM, foo->MOTOR_TEMP, foo->AMBIENT_TEMP, foo->AMP_LOAD, foo->VOLTAGE, foo->LAT, foo->LONG, foo->X_ACEL, foo->Y_ACEL, foo->UTC_HOUR, foo->UTC_MINUTE, foo->UTC_SECOND, foo->LAP, foo->LAP_TIME, foo->TOTAL_TIME, foo->LoRa_SNR);
  dataLog.close();
}
