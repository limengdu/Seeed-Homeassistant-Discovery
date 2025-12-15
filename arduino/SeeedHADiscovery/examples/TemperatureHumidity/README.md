# Temperature & Humidity Sensor Example

Report temperature and humidity data to Home Assistant in real-time. Supports both real DHT22 sensor and simulated data.

## Features

- Temperature sensor (°C)
- Humidity sensor (%)
- Configurable update interval
- Optional DHT22 sensor support
- Simulated data mode for testing

## Hardware Requirements

- XIAO ESP32-C3/C6/S3 or other ESP32 development boards
- DHT22 sensor (optional - can use simulated data)

### DHT22 Wiring

| DHT22 Pin | Connection |
|-----------|------------|
| VCC | 3.3V |
| GND | GND |
| DATA | D2 (configurable) |

## Software Dependencies

### Required Libraries

Install via Arduino Library Manager:

| Library | Author | Description |
|---------|--------|-------------|
| **ArduinoJson** | Benoit Blanchon | JSON parsing |
| **WebSockets** | Markus Sattler | WebSocket communication |
| **DHT sensor library** | Adafruit | DHT22 support (if using real sensor) |

### SeeedHADiscovery Library

Install manually from [GitHub](https://github.com/limengdu/SeeedHADiscovery).

## Quick Start

### 1. Configure WiFi

```cpp
const char* WIFI_SSID = "Your_WiFi_SSID";
const char* WIFI_PASSWORD = "Your_WiFi_Password";
```

### 2. Enable DHT22 (Optional)

To use a real DHT22 sensor, uncomment:
```cpp
#include <DHT.h>
#define USE_DHT_SENSOR
```

### 3. Configure Update Interval

```cpp
const unsigned long UPDATE_INTERVAL = 5000;  // 5 seconds
```

### 4. Upload and Connect

1. Select board: **XIAO ESP32C6** (or your board)
2. Upload the sketch
3. Open Serial Monitor (115200 baud)
4. Add device in Home Assistant

## Home Assistant Setup

1. Go to **Settings** → **Devices & Services** → **Add Integration**
2. Search for **Seeed HA Discovery**
3. Enter the device IP address
4. Two sensors will appear:
   - Temperature
   - Humidity

## Entities Created

| Entity | Type | Unit | Precision |
|--------|------|------|-----------|
| Temperature | Sensor | °C | 1 decimal |
| Humidity | Sensor | % | Integer |

## Simulated Data Mode

When `USE_DHT_SENSOR` is not defined:
- Temperature: Fluctuates between 20-30°C
- Humidity: Fluctuates between 40-70%

This is useful for testing without hardware.

## Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `DHT_PIN` | D2 | DHT22 data pin |
| `DHT_TYPE` | DHT22 | Sensor type (DHT11 or DHT22) |
| `UPDATE_INTERVAL` | 5000ms | Data reporting interval |

## Device Status Page

Access device status at: `http://<device_ip>/`

## Troubleshooting

### DHT22 read fails
- Check wiring connections
- Verify sensor is DHT22 not DHT11
- Ensure 3.3V power supply

### Values not updating in HA
- Check WiFi connection
- Verify device is connected to HA
- Check Serial Monitor for errors

## License

Part of the SeeedHADiscovery library.

