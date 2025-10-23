
#include <SPI.h>
#include <RH_RF95.h>

#define RF95_CS   17
#define RF95_INT  20
#define RF95_RST  21

#define RF95_FREQ 915.0

// Don't pass SPI here, let RadioHead use the default hardware_spi instance
RH_RF95 rf95(RF95_CS, RF95_INT);

int16_t packetnum = 0;

void setup() {
  pinMode(RF95_RST, OUTPUT);
  digitalWrite(RF95_RST, HIGH);

  Serial.begin(115200);

  Serial.println("LoRa TX using default SPI");

  // Reset LoRa module
  digitalWrite(RF95_RST, LOW);
  delay(10);
  digitalWrite(RF95_RST, HIGH);
  delay(10);

  SPI.begin();  // Initialize default SPI bus

  if (!rf95.init()) {
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

  rf95.setTxPower(23, false);
}

void loop() {
  delay(1000);
  Serial.println("Transmitting...");

  char radiopacket[20] = "Hello World #      ";
  itoa(packetnum++, radiopacket + 13, 10);
  radiopacket[19] = 0;

  Serial.print("Sending ");
  Serial.println(radiopacket);

  rf95.send((uint8_t *)radiopacket, 20);
  rf95.waitPacketSent();

  Serial.println("Waiting for reply...");
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (rf95.waitAvailableTimeout(1000)) {
    if (rf95.recv(buf, &len)) {
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
    } else {
      Serial.println("Receive failed");
    }
  } else {
    Serial.println("No reply, is there a listener around?");
  }
}

