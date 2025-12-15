# LED Switch BLE Example

Control an LED from Home Assistant via Bluetooth Low Energy. Features bidirectional GATT communication and BTHome protocol support.

## Features

- Control LED on/off from Home Assistant via BLE
- Bidirectional GATT communication
- BTHome v2 protocol for auto-discovery
- LED state sensor for status feedback
- Ultra-low power operation

## Hardware Requirements

- XIAO ESP32-C3/C6/S3 or XIAO nRF52840
- LED (onboard or external)

### LED Configuration

| Board | Onboard LED | Notes |
|-------|------------|-------|
| XIAO ESP32-S3 | GPIO21 | Has User LED |
| XIAO ESP32-C6 | GPIO15 | Has User LED |
| XIAO ESP32-C3 | N/A | **No User LED** - requires external |
| XIAO nRF52840 | Built-in | Has User LED |

## Software Dependencies

### For ESP32

Install via Arduino Library Manager:

| Library | Description |
|---------|-------------|
| **NimBLE-Arduino** | BLE stack for ESP32 |

### For nRF52840 (mbed)

- **ArduinoBLE** (built-in)

### SeeedHADiscoveryBLE Library

Install manually from [GitHub](https://github.com/limengdu/SeeedHADiscovery).

## Quick Start

### 1. Configure Device Name

```cpp
const char* DEVICE_NAME = "XIAO LED Controller";
```

### 2. Configure External LED (if needed)

For XIAO ESP32-C3:
```cpp
#define EXTERNAL_LED
#define LED_PIN D0
```

### 3. Upload and Discover

1. Upload the sketch
2. Home Assistant will auto-discover via BTHome
3. Device appears as BLE device in HA

## How It Works

```
Home Assistant ──BLE GATT──> Device
     │                         │
     │    Write Command        │
     ▼                         ▼
  [Switch]              [LED Control]
     │                         │
     │    Notify State         │
     ◄─────────────────────────┘
```

1. Device advertises BTHome data
2. HA discovers and connects via GATT
3. HA writes commands to control characteristic
4. Device controls LED and notifies state change

## GATT Service

| UUID | Description |
|------|-------------|
| Control Service | `SEEED_CONTROL_SERVICE_UUID` |
| Command Char | `SEEED_CONTROL_COMMAND_CHAR_UUID` |
| State Char | `SEEED_CONTROL_STATE_CHAR_UUID` |

### Command Format

```
[switch_index][state]
e.g.: 0x00 0x01 = Switch 0 ON
      0x00 0x00 = Switch 0 OFF
```

## Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `DEVICE_NAME` | "XIAO LED Controller" | BLE device name |
| `ADVERTISE_INTERVAL` | 5000ms | BTHome broadcast interval |
| `LED_ACTIVE_LOW` | true | LED polarity |

## Testing with nRF Connect

1. Scan and connect to device
2. Find Control Service
3. Write to Command characteristic:
   - `00 01` to turn ON
   - `00 00` to turn OFF

## Troubleshooting

### Device not discovered
- Ensure BLE is enabled on HA host
- Or use ESPHome Bluetooth Proxy
- Check device is advertising (Serial Monitor)

### LED not responding
- Check LED_ACTIVE_LOW setting
- Verify LED wiring if external

## License

Part of the SeeedHADiscoveryBLE library.

