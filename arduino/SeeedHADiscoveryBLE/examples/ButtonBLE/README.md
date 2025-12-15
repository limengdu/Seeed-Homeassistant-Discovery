# Button BLE Example

Send button events to Home Assistant via Bluetooth Low Energy. Supports single click, double click, triple click, and long press detection.

## Features

- Multiple button event types (single, double, triple, long press)
- Three independent switches for different press types
- BTHome v2 protocol for auto-discovery
- GATT bidirectional communication
- Immediate state broadcast on button press
- Ultra-low power design

## Hardware Requirements

- XIAO ESP32-C3/C6/S3 or XIAO nRF52840
- Button connected to GPIO (default D1)

### Button Wiring

| Button Pin | Connection |
|------------|------------|
| Terminal 1 | GPIO D1 (default) |
| Terminal 2 | GND |

Internal pull-up resistor is enabled.

## Software Dependencies

### For ESP32

Install via Arduino Library Manager:

| Library | Description |
|---------|-------------|
| **NimBLE-Arduino** | BLE stack for ESP32 |

### For nRF52840

- **ArduinoBLE** (mbed) or **Bluefruit** (Adafruit) - built-in

### SeeedHADiscoveryBLE Library

Install manually from [GitHub](https://github.com/limengdu/SeeedHADiscovery).

## Quick Start

### 1. Configure Device Name

```cpp
const char* DEVICE_NAME = "XIAO Button";
```

### 2. Configure Button Pin

```cpp
#define BUTTON_PIN D1
```

### 3. Upload and Use

1. Upload the sketch
2. Home Assistant auto-discovers via BTHome
3. Press button to toggle switches

## Button Events

| Event | Action | Timing |
|-------|--------|--------|
| Single Click | Toggle Single Click switch | Press < 1s, no follow-up |
| Double Click | Toggle Double Click switch | 2 presses within 300ms |
| Triple Click | Toggle Double Click switch | 3 presses within 300ms |
| Long Press | Toggle Long Press switch | Hold > 1 second |

## Entities Created

| Entity | Type | Trigger |
|--------|------|---------|
| Single Click | Switch | Single press |
| Double Click | Switch | Double/Triple press |
| Long Press | Switch | Long press > 1s |

## Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `DEVICE_NAME` | "XIAO Button" | BLE device name |
| `BUTTON_PIN` | D1 | Button GPIO pin |
| `LONG_PRESS_TIME` | 1000ms | Long press threshold |
| `DOUBLE_CLICK_TIME` | 300ms | Double click interval |
| `ADVERTISE_INTERVAL` | 5000ms | BTHome broadcast interval |

## How It Works

1. Button press detected → Analyze press pattern
2. Determine event type (single/double/triple/long)
3. Toggle corresponding switch state
4. Immediately broadcast state via BTHome
5. HA receives and updates entity state

## State Synchronization

- Local button press → Updates switch → Broadcasts to HA
- HA remote control → Updates switch → Syncs sensor state

## Testing

1. Open Serial Monitor (115200 baud)
2. Press button and observe:
   - Event type detected
   - Switch state changes
   - BLE advertisement sent

## Troubleshooting

### Button not responding
- Check wiring (button to GND)
- Verify BUTTON_PIN setting
- Check Serial Monitor for events

### Device not discovered
- Ensure HA has BLE adapter or proxy
- Check device is advertising

### Double click not registering
- Press faster (within 300ms)
- Adjust `DOUBLE_CLICK_TIME` if needed

## License

Part of the SeeedHADiscoveryBLE library.

