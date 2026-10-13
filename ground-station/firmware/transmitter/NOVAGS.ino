#include <SPI.h>
#include <LoRa.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

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

// =========================
// Sensors
// =========================
TinyGPSPlus gps;
Adafruit_BMP280 bmp;
Adafruit_MPU6050 mpu;

// =========================
// Timing
// =========================
unsigned long packetNumber = 0;
unsigned long lastTransmit = 0;


void setup() {

  Serial.begin(115200);
  delay(1000);

  // =========================
  // Feather status LED
  // =========================

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);


  Serial.println();
  Serial.println("==============================");
  Serial.println(" ROCKET GROUND STATION TEST");
  Serial.println("==============================");


  // =========================
  // I2C
  // =========================

  Wire.begin();

  Serial.println("I2C bus started.");


  // =========================
  // GPS
  // =========================

  Serial1.begin(
    9600,
    SERIAL_8N1,
    GPS_RX,
    GPS_TX
  );

  Serial.println("GPS UART started.");


  // =========================
  // BMP280
  // =========================

  if (!bmp.begin(0x76)) {

    Serial.println("ERROR: BMP280 NOT FOUND.");
    Serial.println("Expected address: 0x76");

    while (1) {
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
    }
  }

  Serial.println("BMP280 initialized at 0x76.");

  bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,
    Adafruit_BMP280::SAMPLING_X2,
    Adafruit_BMP280::SAMPLING_X16,
    Adafruit_BMP280::FILTER_X16,
    Adafruit_BMP280::STANDBY_MS_125
  );


  // =========================
  // MPU6050
  // =========================

  if (!mpu.begin(0x68)) {

    Serial.println("ERROR: MPU6050 NOT FOUND.");
    Serial.println("Expected address: 0x68");

    while (1) {
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
    }
  }

  Serial.println("MPU6050 initialized at 0x68.");


  // MPU6050 configuration

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("MPU6050 configured.");


  // =========================
  // LoRa
  // =========================

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


  // =========================
  // READY
  // =========================

  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" ALL SYSTEMS INITIALIZED");
  Serial.println("==============================");
  Serial.println("Waiting for GPS fix...");
  Serial.println();
}


void loop() {

  // =========================
  // READ GPS CONSTANTLY
  // =========================

  while (Serial1.available()) {
    gps.encode(Serial1.read());
  }


  // =========================
  // RUN EVERY 1 SECOND
  // =========================

  if (millis() - lastTransmit >= 100) {

    lastTransmit = millis();


    // =========================
    // GPS DATA
    // =========================

    bool fixValid =
      gps.location.isValid() &&
      gps.location.age() < 5000;


    // =========================
    // BMP280 DATA
    // =========================

    float temperature = bmp.readTemperature();

    float pressure =
      bmp.readPressure() / 100.0F;

    float bmpAltitude =
      bmp.readAltitude(1013.25);


    // =========================
    // MPU6050 DATA
    // =========================

    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;

    mpu.getEvent(
      &accel,
      &gyro,
      &temp
    );


    // =========================
    // LORA TRANSMISSION
    // =========================

    LoRa.beginPacket();

    LoRa.print("ROCKET,");

    LoRa.print(packetNumber);

    LoRa.print(",");


    // GPS FIX

    if (fixValid) {
      LoRa.print("FIX");
    }
    else {
      LoRa.print("NOFIX");
    }

    LoRa.print(",");


    // GPS LAT

    if (gps.location.isValid()) {
      LoRa.print(gps.location.lat(), 6);
    }
    else {
      LoRa.print("0");
    }

    LoRa.print(",");


    // GPS LON

    if (gps.location.isValid()) {
      LoRa.print(gps.location.lng(), 6);
    }
    else {
      LoRa.print("0");
    }

    LoRa.print(",");


    // GPS ALTITUDE

    if (gps.altitude.isValid()) {
      LoRa.print(gps.altitude.meters(), 1);
    }
    else {
      LoRa.print("0");
    }

    LoRa.print(",");


    // SATELLITES

    if (gps.satellites.isValid()) {
      LoRa.print(gps.satellites.value());
    }
    else {
      LoRa.print("0");
    }

    LoRa.print(",");


    // BMP PRESSURE

    LoRa.print(pressure, 2);

    LoRa.print(",");


    // BMP TEMPERATURE

    LoRa.print(temperature, 2);

    LoRa.print(",");


    // BMP ALTITUDE

    LoRa.print(bmpAltitude, 1);

    LoRa.print(",");


    // MPU ACCELERATION

    LoRa.print(accel.acceleration.x, 2);
    LoRa.print(",");
    LoRa.print(accel.acceleration.y, 2);
    LoRa.print(",");
    LoRa.print(accel.acceleration.z, 2);

    LoRa.print(",");


    // MPU GYRO

    LoRa.print(gyro.gyro.x, 2);
    LoRa.print(",");
    LoRa.print(gyro.gyro.y, 2);
    LoRa.print(",");
    LoRa.print(gyro.gyro.z, 2);


    LoRa.endPacket();


    // =========================
    // SERIAL MONITOR
    // =========================

    Serial.print("TX #");
    Serial.print(packetNumber);

    Serial.print(" | ");


    // GPS

    if (fixValid) {

      Serial.print("GPS FIX");

      Serial.print(" | LAT: ");
      Serial.print(gps.location.lat(), 6);

      Serial.print(" | LON: ");
      Serial.print(gps.location.lng(), 6);

      Serial.print(" | GPS ALT: ");
      Serial.print(gps.altitude.meters(), 1);
      Serial.print(" m");

    }
    else {

      Serial.print("NO GPS FIX");
    }


    Serial.print(" | SAT: ");

    if (gps.satellites.isValid()) {
      Serial.print(gps.satellites.value());
    }
    else {
      Serial.print("0");
    }


    // BMP

    Serial.print(" | BMP T: ");
    Serial.print(temperature, 2);
    Serial.print(" C");

    Serial.print(" | PRESS: ");
    Serial.print(pressure, 2);
    Serial.print(" hPa");

    Serial.print(" | BMP ALT: ");
    Serial.print(bmpAltitude, 1);
    Serial.print(" m");


    // MPU ACCEL

    Serial.print(" | ACCEL: ");

    Serial.print(accel.acceleration.x, 2);
    Serial.print(", ");

    Serial.print(accel.acceleration.y, 2);
    Serial.print(", ");

    Serial.print(accel.acceleration.z, 2);

    Serial.print(" m/s^2");


    // MPU GYRO

    Serial.print(" | GYRO: ");

    Serial.print(gyro.gyro.x, 2);
    Serial.print(", ");

    Serial.print(gyro.gyro.y, 2);
    Serial.print(", ");

    Serial.print(gyro.gyro.z, 2);

    Serial.println(" rad/s");


    packetNumber++;
  }
}