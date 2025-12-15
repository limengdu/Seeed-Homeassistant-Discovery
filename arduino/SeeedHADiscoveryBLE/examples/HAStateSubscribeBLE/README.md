# HA State Subscribe BLE Example

Receive Home Assistant entity states via Bluetooth Low Energy. Fully dynamic - no hardcoding required!

## Features

- Subscribe to ANY Home Assistant entities
- Fully dynamic configuration via HA interface
- Real-time state updates via BLE
- Up to 16 entities supported
- Auto-display of entity names
- GATT bidirectional communication

## Hardware Requirements

- XIAO ESP32-C3/C6/S3 or XIAO nRF52840

## Software Dependencies

### For ESP32

Install via Arduino Library Manager:

| Library | Description |
|---------|-------------|
| **NimBLE-Arduino** | BLE stack for ESP32 |

### For nRF52840

- **ArduinoBLE** (mbed) - built-in

### SeeedHADiscoveryBLE Library

Install manually from [GitHub](https://github.com/limengdu/SeeedHADiscovery).

## Quick Start

### 1. Upload Code

Just upload - no configuration needed in code!

```cpp
const char* DEVICE_NAME = "XIAO HA State Monitor";
```

### 2. Configure in Home Assistant

1. Find the BLE device in HA
2. Click **Configure** in device options
3. Select ANY entities you want to receive
4. Save - they will appear automatically!

### 3. View States

States are shown in Serial Monitor and can be used in your code.

## How It Works

```
┌─────────────────┐         ┌─────────────────┐
│  Home Assistant │         │     Device      │
│                 │         │                 │
│  Entity States  │──BLE───▶│  onHAState()    │
│                 │         │    callback     │
│  Configuration  │◀──BLE───│                 │
└─────────────────┘         └─────────────────┘
```

1. Upload code to device
2. Configure subscriptions in HA
3. HA pushes entity states via BLE GATT
4. Device receives and processes states

## Callback Function

```cpp
void onHAStateReceived(uint8_t entityIndex, 
                       const char* entityId, 
                       const char* state, 
                       float numericValue) {
    Serial.print("Entity: ");
    Serial.println(entityId);
    Serial.print("State: ");
    Serial.println(state);
    Serial.print("Value: ");
    Serial.println(numericValue);
}
```

## Accessing States

```cpp
// Get state by index
SeeedBLEHAState* state = ble.getHAState(0);
if (state && state->hasValue()) {
    String entityId = state->getEntityId();
    String stateStr = state->getString();
    float value = state->getFloat();
}

// Get subscribed entity count
uint8_t count = ble.getSubscribedEntityCount();
```

## Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `DEVICE_NAME` | "XIAO HA State Monitor" | BLE device name |
| `ADVERTISE_INTERVAL` | 5000ms | BTHome broadcast interval |
| `MAX_ENTITIES` | 16 | Maximum subscribed entities |

## Use Cases

- Display sensor values on screen
- React to entity changes
- Create portable dashboard
- Build automation triggers

## Example: Temperature Alert

```cpp
void onHAStateReceived(...) {
    if (strstr(entityId, "temperature") != NULL) {
        if (numericValue > 30.0) {
            Serial.println("Temperature is high!");
            // Trigger alarm, LED, etc.
        }
    }
}
```

## Example: Light Status

```cpp
void onHAStateReceived(...) {
    if (strstr(entityId, "light.") != NULL) {
        bool isOn = (strcmp(state, "on") == 0);
        Serial.print("Light is: ");
        Serial.println(isOn ? "ON" : "OFF");
    }
}
```

## Serial Output Format

```
╔══════════════════════════════════════════╗
║       HA State Update Received           ║
╠══════════════════════════════════════════╣
║ Index:  0                                ║
║ Entity: sensor.temperature               ║
║ State:  25.5                             ║
║ Value:  25.50                            ║
╚══════════════════════════════════════════╝
```

## Troubleshooting

### No entities appearing
- Configure subscriptions in HA device settings
- Check BLE connection status

### States not updating
- Verify BLE connection
- Check HA integration status

### Device not discovered
- Ensure HA has BLE adapter or proxy
- Check device is advertising

## License

Part of the SeeedHADiscoveryBLE library.

