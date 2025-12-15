# Temperature & Humidity BLE Example

Broadcast temperature and humidity data to Home Assistant via Bluetooth Low Energy using BTHome v2 protocol.

## Features

- Temperature sensor broadcast
- Humidity sensor broadcast
- BTHome v2 protocol for auto-discovery
- Ultra-low power operation
- Optional DHT22 sensor support
- Simulated data mode for testing

## Hardware Requirements

- XIAO ESP32-C3/C6/S3 or XIAO nRF52840
- DHT22 sensor (optional - can use simulated data)

### DHT22 Wiring

| DHT22 Pin | Connection |
|-----------|------------|
| VCC | 3.3V |
| GND | GND |
| DATA | D2 (configurable) |

## Software Dependencies

### For ESP32

Install via Arduino Library Manager:

| Library | Description |
|---------|-------------|
| **NimBLE-Arduino** | BLE stack for ESP32 |
| **DHT sensor library** | DHT22 support (if using) |

### For nRF52840

- **ArduinoBLE** (mbed) or **Bluefruit** (Adafruit) - built-in
- **DHT sensor library** (if using DHT22)

### SeeedHADiscoveryBLE Library

Install manually from [GitHub](https://github.com/limengdu/SeeedHADiscovery).

## Quick Start

### 1. Configure Device Name

```cpp
const char* DEVICE_NAME = "XIAO Temp/Humidity";
```

### 2. Enable DHT22 (Optional)

To use real sensor:
```cpp
#include <DHT.h>
#define USE_DHT_SENSOR
```

### 3. Upload

1. Upload the sketch
2. Home Assistant auto-discovers via BTHome
3. Temperature and humidity sensors appear in HA

## Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `DEVICE_NAME` | "XIAO Temp/Humidity" | BLE device name |
| `ADVERTISE_INTERVAL` | 10000ms | Broadcast interval (10s) |
| `DHT_PIN` | D2 | DHT22 data pin |
| `DHT_TYPE` | DHT22 | Sensor type |

## Simulated Data Mode

When `USE_DHT_SENSOR` is not defined:
- Temperature: Fluctuates 20-30°C
- Humidity: Fluctuates 40-70%

Great for testing without hardware!

## BTHome Protocol

Data is broadcast using BTHome v2 format:
- Auto-discovered by Home Assistant
- No pairing required
- Works with ESPHome Bluetooth Proxy

## Power Optimization

| Parameter | Effect |
|-----------|--------|
| Longer interval | Lower power consumption |
| Shorter interval | Faster updates |

Recommended: 10-60 seconds for battery operation.

## Entities Created

| Entity | Type | Unit |
|--------|------|------|
| Temperature | Sensor | °C |
| Humidity | Sensor | % |

## Home Assistant Requirements

- Bluetooth adapter on HA host, OR
- ESPHome Bluetooth Proxy
- BTHome integration enabled

## Testing

1. Open Serial Monitor (115200 baud)
2. Observe broadcast messages:
   ```
   Broadcast: Temp=25.0C, Humidity=55%
   ```
3. Check HA for new BTHome device

## Troubleshooting

### Device not discovered
- Ensure HA has BLE capability
- Check device is advertising
- May take a few minutes for discovery

### DHT22 read failed
- Check wiring connections
- Verify 3.3V power supply
- Ensure correct DHT_TYPE setting

### Values not updating
- Check ADVERTISE_INTERVAL
- Verify BLE connection in HA

## License

Part of the SeeedHADiscoveryBLE library.

