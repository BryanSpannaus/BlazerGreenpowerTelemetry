
#include <SPI.h>
#include <RH_RF95.h>

#define RF95_CS   17
#define RF95_INT  20
#define RF95_RST  21

#define STATUS_LED 0
#define DATA_LED 1

#define RF95_FREQ 915.0

// Don't pass SPI here, let RadioHead use the default hardware_spi instance
RH_RF95 rf95(RF95_CS, RF95_INT);

int16_t packetnum = 0;

void setup() {
  pinMode(RF95_RST, OUTPUT);
  digitalWrite(RF95_RST, HIGH);
  pinMode(STATUS_LED, OUTPUT);
  pinMode(DATA_LED, OUTPUT);
  Serial.begin(115200);
  
  delay(5000); //waits 5 seconds before starting output and setup after plugging in device

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
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(DATA_LED, HIGH);

      RH_RF95::printBuffer("Received bytes: ", buf, len);
      Serial.println((char*)buf);
      
      uint8_t data[] = "RCV_CONF";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();

      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(DATA_LED, LOW);
    } else {
      Serial.println("RX Fail");
    }
  }
}
