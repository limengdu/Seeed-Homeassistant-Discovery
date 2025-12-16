# HA State Subscribe Example

Receive and display entity states from Home Assistant on your device. Perfect for creating dashboard displays or reacting to changes in other HA entities.

## Features

- Subscribe to any Home Assistant entity
- Two operation modes: Push (event-driven) and Polling (timer-based)
- Real-time state updates
- Access to entity attributes (friendly name, unit, device class)
- Dynamic entity configuration via HA interface

## Use Cases

- Display sensor values on a screen
- React to changes in other HA entities
- Create a dashboard device
- Build automation triggers

## Hardware Requirements

- XIAO ESP32-C3/C5/C6/S3 or other ESP32 development boards
- Optional: Display for showing states

> **Note**: XIAO ESP32-C5 supports both 2.4GHz and 5GHz dual-band WiFi

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
const char* WIFI_SSID = "your-wifi-ssid";
const char* WIFI_PASSWORD = "your-wifi-password";
```

### 2. Upload and Connect

1. Upload the sketch to your device
2. Open Serial Monitor (115200 baud)
3. Add device in Home Assistant

### 3. Configure Subscriptions in HA

1. Find your device in **Settings** → **Devices & Services**
2. Click **Configure** on your device
3. Select entities you want to subscribe to
4. Save the configuration

## Two Operation Modes

### Mode 1: Push Mode (Event-driven) - Recommended

```cpp
ha.onHAState([](const char* entityId, const char* state, JsonObject& attrs) {
    Serial.print("Entity: ");
    Serial.println(entityId);
    Serial.print("State: ");
    Serial.println(state);
});
```

- HA automatically pushes updates when entities change
- Best for: Real-time reactions, logging, alerts

### Mode 2: Polling Mode (Timer-based)

```cpp
SeeedHAState* temp = ha.getHAState("sensor.temperature");
if (temp && temp->hasValue()) {
    float value = temp->getFloat();
    // Use the value
}
```

- Read stored states on your schedule
- Best for: Screen refresh, periodic display updates

## Available Methods for SeeedHAState

| Method | Return Type | Description |
|--------|-------------|-------------|
| `getString()` | String | State as string |
| `getFloat()` | float | State as number |
| `getBool()` | bool | State as boolean |
| `hasValue()` | bool | Check if value exists |
| `getEntityId()` | String | Entity ID |
| `getFriendlyName()` | String | Display name |
| `getUnit()` | String | Unit of measurement |
| `getDeviceClass()` | String | Device class |

## Example: Dashboard Display

```cpp
void refreshScreen() {
    for (const auto& pair : ha.getHAStates()) {
        SeeedHAState* state = pair.second;
        if (state->hasValue()) {
            String line = state->getFriendlyName() + ": " + state->getString();
            // Draw to display
        }
    }
}
```

## Example: Conditional Logic

```cpp
SeeedHAState* temp = ha.getHAState("sensor.temperature");
if (temp && temp->hasValue() && temp->getFloat() > 28.0) {
    Serial.println("Warning: Temperature too high!");
}
```

## Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `PRINT_INTERVAL` | 5000ms | Status print interval |

## Troubleshooting

### No entities appearing
- Configure subscriptions in HA device settings
- Check Serial Monitor for connection status

### States not updating
- Verify HA WebSocket connection
- Check if entities exist in HA

## License

Part of the SeeedHADiscovery library.

