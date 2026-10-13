# NOVA Ground Station

NOVA is a rocket telemetry and visualization system for the L2 rocket project and its L2 certification. It combines an onboard sensor and LoRa transmitter, an ESP32 ground-station receiver, a browser dashboard, OpenRocket design files, and a 3D rocket model.

## Project Overview

The onboard system sends GPS, barometric, and inertial telemetry over LoRa. The ground receiver forwards decoded values to a laptop over USB serial, where the dashboard displays live instrumentation and the rocket model. The dashboard is for observation and telemetry only; it does not command rocket recovery deployment.

## System Architecture

1. The rocket ESP32 reads the GPS, BMP280, and MPU6050 and transmits packets over LoRa.
2. The ESP32-WROOM ground station receives packets, tracks packet counts, and prints formatted telemetry over USB serial at 115200 baud.
3. The browser dashboard reads that serial stream through the Web Serial API and updates its displays, charts, and 3D view.

## Hardware

**Rocket:** ESP32, 915 MHz LoRa radio, MPU6050 accelerometer/gyroscope, BMP280 barometer, and GPS receiver.

**Ground station:** ESP32-WROOM, 915 MHz LoRa radio, and a USB serial connection to the laptop at 115200 baud.

The repository includes the current NOVAGS sensor transmitter sketch, an additional GPS/LoRa transmitter sketch, and the ground-station receiver sketch.

## Telemetry

The receiver and dashboard handle:

- Latitude, longitude, GPS altitude, GPS fix, and satellite count
- BMP280 temperature, barometric pressure, and barometric altitude
- MPU6050 acceleration X/Y/Z and gyro X/Y/Z
- Estimated roll, pitch, and yaw (yaw is gyro-integrated)
- LoRa RSSI and SNR
- Received and lost packet counters

The radio sketches configure 915 MHz, spreading factor 7, 125 kHz bandwidth, coding rate 4/5, sync word `0x12`, and CRC.

## Ground Station

The dashboard is implemented in HTML, CSS, and JavaScript. It uses the Web Serial API to connect to the receiver at 115200 baud and displays telemetry, flight-state indicators, GPS data, and real-time instrumentation.

## 3D Visualization

The dashboard loads `RocketModel.glb` with Three.js. Chart.js is used for telemetry charts. `index.html` is a copy of the current dashboard file; the original filename is preserved alongside it.

## OpenRocket

The available OpenRocket files are organized by level. The latest available L2 design is `openrocket/L2/Mega-Initiator.ork` (internal design name “Mega Initiator”); an earlier saved version is also included. The L1 design is `openrocket/L1/Mega-InitiatorL1.ork`.

## CAD

Onshape document: [Avionics/Payload Bay](https://cad.onshape.com/documents/566e8e0aca21c52b1c66a7da/w/7f8e277643475ee5df21b529/e/8856235a72fec4b5fb20d49c?renderMode=0&uiState=6acdc1caae09b9b1060bae04)

The link opens the document in view-only mode. No STEP or STL exports are included because export access was not available from that view.

## Repository Structure

```text
NOVA-Ground-Station/
├── ground-station/
│   ├── index.html
│   ├── nova-telemetry-dashboard-model-v2.html
│   ├── RocketModel.glb
│   └── firmware/
│       ├── receiver/GroundStation.ino
│       └── transmitter/
│           ├── NOVAGS.ino
│           └── Transmitter.ino
└── openrocket/
    ├── L1/Mega-InitiatorL1.ork
    └── L2/
        ├── Mega-Initiator copy.ork
        └── Mega-Initiator.ork
```

## Running the Ground Station

From the repository root, serve the dashboard files locally:

```sh
cd ground-station
python3 -m http.server 8000
```

Then open [http://localhost:8000/](http://localhost:8000/) in Chrome or Edge. Web Serial requires a supported browser and a secure context; localhost is supported. Close Arduino Serial Monitor before connecting the browser to the ESP32, since only one application can use the serial port at a time.
