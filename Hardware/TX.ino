#include <SPI.h>
#include <RH_RF95.h> // Version 1.121
#include <cmath>
#include <cstdint>
#include <vector>


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

//Lat and Long transmit points
#define TRANSMISSION_LONG
#define TRANSMISSION_LAT

//Data Structures
struct HEADER {
  float SYSBAT;
  int UID;
};

class DATA_BUFFER {
  public:
  float SEND_RPM; //RPMs
  float BATT_TEMP; 
  float MOTOR_TEMP;
  float AMBIENT_TEMP;
  float AMP_LOAD;
  float VOLTAGE;
  float LAT; //GPS data
  float LONG;
  float SPEED;
  float X_ROT; //Gyroscope data
  float Y_ROT;
  float Z_ROT;
  float X_ACEL; //Accelerometer data
  float Y_ACEL;
  float Z_ACEL;
  int UTC_HOUR; //UTC time from GPS
  int UTC_MINUTE;
  int UTC_SECOND;
  int LAP; //# of laps calculated by # of buffers cleared +1
  int LAP_TIME;
  int TOTAL_TIME; 
};


void addPoint(std::vector<DATA_BUFFER>*, int, struct DATA_BUFFER*);

//RPM Calculating Variables (used in hallISR, and Loop)
volatile unsigned long lastPulseTime = 0;
volatile unsigned long pulseInterval = 0;
volatile bool newPulse = false;


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

  attachInterrupt(digitalPinToInterrupt(RPM_SENSE), hallISR, FALLING);

  Serial.begin(115200);

  Serial.println("LoRa TX using default SPI");

  // Reset LoRa module
  digitalWrite(RF95_RST, LOW);
  delay(10);
  digitalWrite(RF95_RST, HIGH);
  delay(10);

  SPI.begin(); 

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

  digitalWrite(BUZZER, HIGH);
  delay(150);
  digitalWrite(BUZZER, LOW);
}

void loop() {
  int lastUpdateTime;
  float battVoltage;

  bool dataSet; //which struct 0 or 1

  std::vector<struct DATA_BUFFER> buf1[440];
  std::vector<struct DATA_BUFFER> buf0[440];
  struct HEADER header;
  struct DATA_BUFFER temp;


  unsigned long lastCalcTime = 0;
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

  // Print RPM every 500 ms
  if (millis() - lastCalcTime >= 500) {
    lastCalcTime = millis();
    Serial.print("RPM: ");
    Serial.println(rpm, 1);
  }


  battVoltage = (analogRead(VOLTAGE_DIVIDER) * 24.8)/1005;

/*

sprintf(radiopacket, "%f, %i,", rpm, AMPS);
Serial.println(radiopacket);
*/
  //rf95.send((uint8_t *)radiopacket, 127);
  rf95.waitPacketSent();
  

  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

if (millis() >= lastUpdateTime + 1000) {
  lastUpdateTime = millis();
  if(dataSet == false) {
  //addPoint(*data0);
  } else {
  //addPoint(*data1);
 }
}

}

void transmit() {

  
}

void addPoint(std::vector<DATA_BUFFER> s, int idx, DATA_BUFFER* foo){
  s[idx].SEND_RPM = foo->SEND_RPM;
  s[idx].BATT_TEMP = foo->BATT_TEMP;
  s[idx].MOTOR_TEMP = foo->MOTOR_TEMP;
  s[idx].AMBIENT_TEMP = foo->AMBIENT_TEMP;
  s[idx].AMP_LOAD = foo->AMP_LOAD;
  s[idx].VOLTAGE = foo->VOLTAGE;
  s[idx].LAT = foo->LAT;
  s[idx].LONG = foo->LONG;
  s[idx].SPEED = foo->SPEED;
  s[idx].X_ROT = foo->X_ROT;
  s[idx].Y_ROT = foo->Y_ROT;
  s[idx].Z_ROT = foo->Z_ROT;
  s[idx].X_ACEL = foo->X_ACEL;
  s[idx].Y_ACEL = foo->Y_ACEL;
  s[idx].Z_ACEL = foo->Z_ACEL;
  s[idx].UTC_HOUR = foo->UTC_HOUR;
  s[idx].UTC_MINUTE = foo->UTC_MINUTE;
  s[idx].UTC_SECOND = foo->UTC_SECOND;
  s[idx].LAP = foo->LAP;
  s[idx].LAP_TIME = foo->LAP_TIME;
  s[idx].TOTAL_TIME = foo->TOTAL_TIME;
}
