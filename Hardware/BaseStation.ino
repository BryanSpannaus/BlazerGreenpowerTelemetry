
#include <SPI.h>
#include <RH_RF95.h>
#include <stream.h>

#define RF95_CS   17
#define RF95_INT  20
#define RF95_RST  21

#define STATUS_LED 0
#define DATA_LED 1

#define RF95_FREQ 915.0

//Constants
#define TireCirc 20.7

int millisLast = 0;

// Don't pass SPI here, let RadioHead use the default hardware_spi instance
RH_RF95 rf95(RF95_CS, RF95_INT);

int16_t packetnum = 0;

void setup() {
  pinMode(RF95_RST, OUTPUT);
  digitalWrite(RF95_RST, HIGH);
  pinMode(STATUS_LED, OUTPUT);
  pinMode(DATA_LED, OUTPUT);
  Serial.begin(115200);

  // Reset LoRa module
  digitalWrite(RF95_RST, LOW);
  delay(10);
  digitalWrite(RF95_RST, HIGH);
  delay(10);

  digitalWrite(STATUS_LED, HIGH); //Turn on Green Led

  SPI.begin();  // Initialize default SPI bus

  if (!rf95.init()) {
    Serial.println("Init Fail");
    while (1);
    digitalWrite(STATUS_LED, LOW);
  }
  Serial.println("Init Ok");

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
    digitalWrite(STATUS_LED, LOW);
  }
  Serial.print("Set Freq to: ");
  Serial.println(RF95_FREQ);

  rf95.setTxPower(23, false);
}

void loop() {
  //float data[4]; //4 live data points Amps, volts, rpm, temp

  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(DATA_LED, HIGH);
      //RH_RF95::printBuffer("Received bytes: ", buf, len);
      /*
      data[0] = std::stof();
      data[1] =
      data[2] = 
      data[3] = 
      */
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(DATA_LED, LOW);
    } else {
      Serial.println("RX Fail");
    }

  //float speed;
  //speed = ((data[2]*TireCirc)*60)/63360; //Multiply RPMs by circumference (in inches) to get in/s estimate, then multiply by 60 to get in/hr, then divide by 63360 to get mph

  if ((millisLast + 1000) <= millis()) {
    /*
    millisLast = millis();
    Serial.printf("Amp Load: %f.2 \n", data[0]);
    Serial.printf("Volts: %f.2 \n", data[1]);
    Serial.printf("RPM: %f.1 \n", data[2]);
    Serial.printf("Motor Temp: %f.1 C \n", data[3]);
    Serial.printf("Estimated Speed: %f.1 MPH \n", speed);
    Serial.printf("SNR: %i \n", rf95.lastSNR());
    */
  
    Serial.println((char*)buf);
    Serial.println("dBm (Siginal Strength)");
    Serial.println(rf95.lastSNR());

      }

      
  }
}
