# Button Switch Example

A multi-click button detection example that creates three independent switches in Home Assistant, each triggered by different button press patterns.

## Features

- **Single Click**: Toggle Switch 1
- **Double Click**: Toggle Switch 2
- **Long Press (>1s)**: Toggle Switch 3
- Bidirectional control (button ↔ Home Assistant)
- Real-time state synchronization

## Hardware Requirements

- XIAO ESP32-C3/C6/S3 or other ESP32 development boards
- Button (with internal or external pull-up resistor)

### Button Wiring

| Button Pin | Connection |
|------------|------------|
| Terminal 1 | GPIO D1 (default) |
| Terminal 2 | GND |

Internal pull-up resistor is enabled by default.

## Software Dependencies

### Required Libraries

Install via Arduino Library Manager:

| Library | Author | Description |
|---------|--------|-------------|
| **ArduinoJson** | Benoit Blanchon | JSON parsing |
| **WebSockets** | Markus Sattler | WebSocket communication |

### SeeedHADiscovery Library

Install manually from [GitHub](https://github.com/limengdu/SeeedHADiscovery).

## Quick Start

### 1. Configure WiFi

```cpp
const char* WIFI_SSID = "Your_WiFi_SSID";
const char* WIFI_PASSWORD = "Your_WiFi_Password";
```

### 2. Configure Button Pin (Optional)

```cpp
#define BUTTON_PIN D1  // Change if needed
```

### 3. Upload and Connect

1. Select board: **XIAO ESP32C6** (or your board)
2. Upload the sketch
3. Open Serial Monitor (115200 baud) to view IP address
4. Add device in Home Assistant

## Home Assistant Setup

1. Go to **Settings** → **Devices & Services** → **Add Integration**
2. Search for **Seeed HA Discovery**
3. Enter the device IP address
4. Three switches will appear:
   - Single Click
   - Double Click
   - Long Press

## Button Timing Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `LONG_PRESS_TIME` | 1000ms | Long press threshold |
| `DOUBLE_CLICK_TIME` | 300ms | Double click interval |

## Entities Created

| Entity | Type | Icon |
|--------|------|------|
| Single Click | Switch | `mdi:gesture-tap` |
| Double Click | Switch | `mdi:gesture-double-tap` |
| Long Press | Switch | `mdi:gesture-tap-hold` |

## How It Works

1. Press button → Detect press type (single/double/long)
2. Toggle corresponding switch state
3. State syncs to Home Assistant automatically
4. Home Assistant can also control switches remotely

## Troubleshooting

### Button not responding
- Check wiring (button should connect GPIO to GND)
- Verify pull-up resistor is enabled
- Check Serial Monitor for debug output

### WiFi connection fails
- Verify SSID and password
- Check WiFi signal strength

## License

Part of the SeeedHADiscovery library.

