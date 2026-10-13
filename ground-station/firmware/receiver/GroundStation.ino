#include <SPI.h>
#include <LoRa.h>

// ============================================================
// NOVA GROUND STATION RECEIVER
// ESP32-WROOM
// Receives telemetry from rocket
// Outputs clean data over USB Serial for NOVA dashboard
// ============================================================


// =========================
// LoRa pins
// =========================

// CHANGE THESE ONLY IF YOUR GROUND-STATION WIRING IS DIFFERENT

#define LORA_SS    33
#define LORA_RST   27
#define LORA_DIO0  26

#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23


// =========================
// Packet tracking
// =========================

unsigned long packetsReceived = 0;
unsigned long packetsLost = 0;

unsigned long lastPacketNumber = 0;
bool firstPacketReceived = false;


// ============================================================
// Split comma-separated LoRa packet
// ============================================================

int splitCSV(
  const String &input,
  String fields[],
  int maxFields
) {

  int count = 0;
  int start = 0;

  for (int i = 0; i <= input.length(); i++) {

    if (
      i == input.length() ||
      input.charAt(i) == ','
    ) {

      if (count >= maxFields) {
        break;
      }

      fields[count] =
        input.substring(start, i);

      fields[count].trim();

      count++;

      start = i + 1;
    }
  }

  return count;
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" NOVA GROUND STATION");
  Serial.println(" ESP32-WROOM");
  Serial.println("==============================");


  // =========================
  // SPI
  // =========================

  SPI.begin(
    LORA_SCK,
    LORA_MISO,
    LORA_MOSI,
    LORA_SS
  );


  // =========================
  // LoRa
  // =========================

  LoRa.setPins(
    LORA_SS,
    LORA_RST,
    LORA_DIO0
  );


  if (!LoRa.begin(915E6)) {

    Serial.println(
      "# ERROR: LORA INITIALIZATION FAILED"
    );

    while (true) {
      delay(1000);
    }
  }


  // MUST MATCH THE ROCKET TRANSMITTER

  LoRa.setSpreadingFactor(7);

  LoRa.setSignalBandwidth(
    125E3
  );

  LoRa.setCodingRate4(5);

  LoRa.setSyncWord(
    0x12
  );

  LoRa.enableCrc();


  Serial.println(
    "# LoRa initialized at 915 MHz"
  );

  Serial.println(
    "# WAITING FOR ROCKET"
  );

  Serial.println();
}


// ============================================================
// LOOP
// ============================================================

void loop() {

  int packetSize =
    LoRa.parsePacket();


  if (!packetSize) {
    return;
  }


  // ==========================================================
  // Read packet
  // ==========================================================

  String packet = "";

  while (LoRa.available()) {

    packet +=
      (char)LoRa.read();
  }


  // Only accept rocket telemetry

  if (!packet.startsWith("ROCKET,")) {
    return;
  }


  // ==========================================================
  // Split packet
  // ==========================================================

  String field[16];

  int fieldCount =
    splitCSV(
      packet,
      field,
      16
    );


  if (fieldCount != 16) {

    Serial.print(
      "# INVALID PACKET: "
    );

    Serial.println(packet);

    return;
  }


  // ==========================================================
  // Decode telemetry
  // ==========================================================

  unsigned long packetNumber =
    field[1].toInt();


  bool gpsFix =
    field[2] == "FIX";


  double latitude =
    field[3].toDouble();


  double longitude =
    field[4].toDouble();


  float gpsAltitude =
    field[5].toFloat();


  int satellites =
    field[6].toInt();


  // Your transmitter sends pressure before temperature

  float pressure =
    field[7].toFloat();


  float bmpTemperature =
    field[8].toFloat();


  float bmpAltitude =
    field[9].toFloat();


  // MPU acceleration

  float accelX =
    field[10].toFloat();

  float accelY =
    field[11].toFloat();

  float accelZ =
    field[12].toFloat();


  // MPU gyro

  float gyroX =
    field[13].toFloat();

  float gyroY =
    field[14].toFloat();

  float gyroZ =
    field[15].toFloat();


  // ==========================================================
  // Radio statistics
  // ==========================================================

  int rssi =
    LoRa.packetRssi();


  float snr =
    LoRa.packetSnr();


  packetsReceived++;


  // ==========================================================
  // Packet loss
  // ==========================================================

  if (firstPacketReceived) {

    if (
      packetNumber >
      lastPacketNumber + 1
    ) {

      packetsLost +=
        packetNumber -
        lastPacketNumber -
        1;
    }
  }

  else {

    firstPacketReceived = true;
  }


  lastPacketNumber =
    packetNumber;


  // ==========================================================
  // Output to laptop / website
  // ==========================================================

  Serial.print("TX #");
  Serial.print(packetNumber);


  // GPS status

  if (gpsFix) {

    Serial.print(
      " | GPS FIX"
    );

  }

  else {

    Serial.print(
      " | NO FIX"
    );
  }


  // GPS

  Serial.print(
    " | LAT: "
  );

  Serial.print(
    latitude,
    6
  );


  Serial.print(
    " | LON: "
  );

  Serial.print(
    longitude,
    6
  );


  Serial.print(
    " | GPS ALT: "
  );

  Serial.print(
    gpsAltitude,
    1
  );

  Serial.print(
    " m"
  );


  Serial.print(
    " | SAT: "
  );

  Serial.print(
    satellites
  );


  // BMP280

  Serial.print(
    " | BMP T: "
  );

  Serial.print(
    bmpTemperature,
    2
  );

  Serial.print(
    " C"
  );


  Serial.print(
    " | PRESS: "
  );

  Serial.print(
    pressure,
    2
  );

  Serial.print(
    " hPa"
  );


  Serial.print(
    " | BMP ALT: "
  );

  Serial.print(
    bmpAltitude,
    1
  );

  Serial.print(
    " m"
  );


  // MPU6050 acceleration

  Serial.print(
    " | ACCEL: "
  );

  Serial.print(
    accelX,
    2
  );

  Serial.print(", ");

  Serial.print(
    accelY,
    2
  );

  Serial.print(", ");

  Serial.print(
    accelZ,
    2
  );

  Serial.print(
    " m/s^2"
  );


  // MPU6050 gyro

  Serial.print(
    " | GYRO: "
  );

  Serial.print(
    gyroX,
    2
  );

  Serial.print(", ");

  Serial.print(
    gyroY,
    2
  );

  Serial.print(", ");

  Serial.print(
    gyroZ,
    2
  );

  Serial.print(
    " rad/s"
  );


  // Radio

  Serial.print(
    " | RSSI: "
  );

  Serial.print(
    rssi
  );


  Serial.print(
    " | SNR: "
  );

  Serial.print(
    snr,
    1
  );


  Serial.print(
    " | RX: "
  );

  Serial.print(
    packetsReceived
  );


  Serial.print(
    " | LOST: "
  );

  Serial.println(
    packetsLost
  );
}