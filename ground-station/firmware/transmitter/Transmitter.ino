#include <SPI.h>
#include <LoRa.h>
#include <TinyGPSPlus.h>

// =========================
// LoRa pins
// =========================
#define LORA_SS    33
#define LORA_RST   27
#define LORA_DIO0  32

// =========================
// GPS pins
// =========================
#define GPS_RX 7     // Feather RX <- GPS TX
#define GPS_TX 8     // Feather TX -> GPS RX

TinyGPSPlus gps;

unsigned long packetNumber = 0;
unsigned long lastTransmit = 0;

void setup() {

  Serial.begin(115200);
  delay(1000);

  // -------------------------
  // Feather status LED
  // -------------------------
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.println();
  Serial.println("==========================");
  Serial.println(" ROCKET GPS + LoRa TRACKER");
  Serial.println("==========================");

  // -------------------------
  // GPS
  // -------------------------
  Serial1.begin(
    9600,
    SERIAL_8N1,
    GPS_RX,
    GPS_TX
  );

  Serial.println("GPS UART started.");

  // -------------------------
  // LoRa
  // -------------------------
  SPI.begin(
    5,          // SCK
    21,         // MISO
    19,         // MOSI
    LORA_SS
  );

  LoRa.setPins(
    LORA_SS,
    LORA_RST,
    LORA_DIO0
  );

  if (!LoRa.begin(915E6)) {

    Serial.println("ERROR: LoRa initialization FAILED.");

    // LED stays OFF if radio failed
    digitalWrite(LED_BUILTIN, LOW);

    while (1) {
      delay(1000);
    }
  }

  LoRa.setTxPower(10);
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0x12);
  LoRa.enableCrc();

  Serial.println("LoRa initialized at 915 MHz.");

  // TRACKER IS RUNNING
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println("TRACKER READY - LED ON");
  Serial.println("Waiting for GPS...");
}


void loop() {

  // Constantly read GPS
  while (Serial1.available()) {
    gps.encode(Serial1.read());
  }

  // Send packet every 1 second
  if (millis() - lastTransmit >= 1000) {

    lastTransmit = millis();

    bool fixValid =
      gps.location.isValid() &&
      gps.location.age() < 5000;

    // =========================
    // TRANSMIT
    // =========================

    LoRa.beginPacket();

    LoRa.print("ROCKET,");
    LoRa.print(packetNumber);
    LoRa.print(",");

    // GPS fix status
    if (fixValid) {
      LoRa.print("FIX,");
    }
    else {
      LoRa.print("NOFIX,");
    }

    // Latitude
    if (gps.location.isValid()) {
      LoRa.print(gps.location.lat(), 6);
    }
    else {
      LoRa.print("0");
    }

    LoRa.print(",");

    // Longitude
    if (gps.location.isValid()) {
      LoRa.print(gps.location.lng(), 6);
    }
    else {
      LoRa.print("0");
    }

    LoRa.print(",");

    // Altitude in metres
    if (gps.altitude.isValid()) {
      LoRa.print(gps.altitude.meters(), 1);
    }
    else {
      LoRa.print("0");
    }

    LoRa.print(",");

    // Satellites
    if (gps.satellites.isValid()) {
      LoRa.print(gps.satellites.value());
    }
    else {
      LoRa.print("0");
    }

    LoRa.endPacket();

    // =========================
    // SERIAL MONITOR
    // =========================

    Serial.print("TX #");
    Serial.print(packetNumber);
    Serial.print(" | ");

    if (fixValid) {

      Serial.print("GPS FIX");

      Serial.print(" | LAT: ");
      Serial.print(gps.location.lat(), 6);

      Serial.print(" | LON: ");
      Serial.print(gps.location.lng(), 6);

      Serial.print(" | ALT: ");
      Serial.print(gps.altitude.meters(), 1);
      Serial.print(" m");

    }
    else {

      Serial.print("NO GPS FIX");
    }

    Serial.print(" | SAT: ");

    if (gps.satellites.isValid()) {
      Serial.println(gps.satellites.value());
    }
    else {
      Serial.println("0");
    }

    packetNumber++;
  }
}