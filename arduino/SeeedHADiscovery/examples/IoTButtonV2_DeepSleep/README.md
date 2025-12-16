# IoT Button V2 Deep Sleep Example

A low-power IoT button implementation with deep sleep capability for ESP32-C6, featuring Home Assistant integration via the SeeedHADiscovery library.

## Features

### 📶 WiFi Provisioning (NEW!)
- **Web-based Configuration**: No need to hardcode WiFi credentials
- **Captive Portal**: On first boot, device creates AP "Seeed_IoT_Button_V2_AP"
- **Easy Setup**: Connect to AP, open browser, select network, enter password
- **Credential Persistence**: WiFi settings saved to flash, survives reboot
- **Long Press Reset**: In provisioning mode, long press clears credentials

### 🔘 Multi-Click Button Detection
- **Single Click**: Toggle Switch 1 + Pink-purple blink effect
- **Double Click**: Toggle Switch 2 + Orange subtle flicker effect
- **Triple Click**: Toggle Developer Mode (3-minute sleep timeout for firmware upload)
- **Long Press (1-5s)**: Toggle Switch 3 + Rainbow gradient effect
- **Long Press (6s+)**: Reset WiFi credentials and start AP mode for re-provisioning

### 🔋 Battery Monitoring
- Real-time battery voltage measurement via ADC
- Battery percentage calculation (2.75V = 0%, 4.2V = 100%)
- Anti-jump filter to prevent percentage fluctuations
- Persistent storage of last battery percentage

### 💤 Deep Sleep Mode
- Ultra-low power consumption (~10µA in deep sleep)
- GPIO wake-up triggered by button press
- Smart sleep timeout:
  - **HA Connected**: 10 seconds of inactivity
  - **HA Not Connected**: 3 minutes of inactivity
  - **Developer Mode**: 3 minutes (for firmware upload)
- Sleep timeout resets on every button action

### 🌈 RGB LED Effects
- **Blink**: Pink-purple blinking (Switch 1)
- **Subtle Flicker**: Orange breathing effect (Switch 2)
- **Rainbow**: Smooth color gradient (Switch 3)
- **Random Color**: Random color transitions (Dev Mode)

### 🏠 Home Assistant Integration
- Auto-discovery via WebSocket
- Entities exposed:
  - Battery Voltage (sensor)
  - Battery Percentage (sensor)
  - Button State (sensor)
  - Switch 1/2/3 (switch)
- Bidirectional control (device ↔ Home Assistant)
- State persistence across deep sleep cycles

## Hardware Requirements

### Platform
- **MCU**: ESP32-C6 (esp32-c6-devkitc-1)
- **Flash**: 4MB
- **CPU**: 80MHz (optimized for low power)

### Pin Configuration

| Pin | Function | Description |
|-----|----------|-------------|
| GPIO0 | Output | Battery voltage detection enable (HIGH = enabled) |
| GPIO1 | ADC | Battery voltage input (12dB attenuation, ×4.0 multiplier) |
| GPIO2 | Input | Button (pull-up, inverted logic, wake source) |
| GPIO3 | Output | Blue LED (inverted, LOW = ON) |
| GPIO14 | Output | Red LED (inverted, LOW = ON) |
| GPIO18 | Output | LED strip power enable (HIGH = enabled) |
| GPIO19 | Output | WS2812 RGB LED data (single LED, GRB order) |

### Circuit Notes
- Button connects GPIO2 to GND (internal pull-up enabled)
- LEDs use inverted logic (LOW = ON, HIGH = OFF)
- Battery voltage divider requires 4.0× multiplier for actual voltage

## Software Dependencies

### Required Libraries

Install the following libraries via Arduino Library Manager (**Sketch** → **Include Library** → **Manage Libraries...**):

| Library | Author | Version | Notes |
|---------|--------|---------|-------|
| **ArduinoJson** | Benoit Blanchon | ≥ 6.x | JSON parsing for HA communication |
| **WebSockets** | Markus Sattler | ≥ 2.x | WebSocket client/server |
| **Adafruit NeoPixel** | Adafruit | ≥ 1.x | WS2812 RGB LED control |

### Built-in Libraries (No Installation Required)

These libraries are included with ESP32 Arduino Core:

- **Preferences** - Non-volatile storage
- **WiFi** - WiFi connectivity
- **esp_sleep.h** - Deep sleep functions
- **driver/gpio.h** - GPIO control

### SeeedHADiscovery Library

This library needs to be installed manually:

1. Download from [GitHub](https://github.com/limengdu/SeeedHADiscovery)
2. In Arduino IDE: **Sketch** → **Include Library** → **Add .ZIP Library...**
3. Select the downloaded ZIP file

Or clone to your Arduino libraries folder:
```bash
cd ~/Documents/Arduino/libraries
git clone https://github.com/limengdu/SeeedHADiscovery.git
```

### ESP32 Board Package

Make sure you have the ESP32 board package installed:

1. Open **File** → **Preferences**
2. Add to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Open **Tools** → **Board** → **Boards Manager...**
4. Search for "esp32" and install **esp32 by Espressif Systems** (version ≥ 3.x recommended)

## Quick Start

### 1. Configure WiFi (Choose One Method)

#### Option A: Web-based Provisioning (Recommended)

No code changes needed! WiFi is configured via web browser:

1. Upload the sketch (with `USE_WIFI_PROVISIONING` set to `true`)
2. On first boot, device creates AP: **Seeed_IoT_Button_V2_AP**
3. Connect your phone/computer to this AP
4. Browser opens automatically, or navigate to `http://192.168.4.1`
5. Select your WiFi network and enter password
6. Device saves credentials and restarts

#### Option B: Hardcoded Credentials

Set `USE_WIFI_PROVISIONING` to `false` and edit the credentials:

```cpp
#define USE_WIFI_PROVISIONING false

const char* WIFI_SSID = "Your_WiFi_SSID";
const char* WIFI_PASSWORD = "Your_WiFi_Password";
```

### 2. Upload the Sketch

1. Select board: **XIAO ESP32C6**
2. Configure settings:
   - Flash Size: 4MB
   - CPU Frequency: 80MHz (recommended for low power)
   - **Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"** ⚠️ Important!
   - Upload Speed: 921600
3. Upload the sketch

> ⚠️ **Important**: If you see the error `text section exceeds available space in board`, you must change the Partition Scheme. Go to **Tools** → **Partition Scheme** and select **"Huge APP (3MB No OTA/1MB SPIFFS)"** or **"Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)"**.

### 3. Add to Home Assistant

1. Open Home Assistant
2. Go to **Settings** → **Devices & Services** → **Add Integration**
3. Search for **Seeed HA Discovery**
4. Enter the device IP address (shown in Serial Monitor)
5. Complete the setup

## LED Status Indicators

| LED | State | Meaning |
|-----|-------|---------|
| Red | ON | WiFi disconnected / Connecting |
| Blue | ON | WiFi connected |
| Red + Blue | Alternating (5x) | WiFi provisioning mode active |
| Red + Blue | Blinking (3x) | Developer mode enabled |
| RGB | Various effects | Button action feedback |

## WiFi Provisioning Mode

When `USE_WIFI_PROVISIONING` is enabled (default), the device uses web-based WiFi configuration:

### First Boot / No Credentials
1. Device creates AP hotspot: **Seeed_IoT_Button_V2_AP**
2. Red/Blue LEDs alternate to indicate provisioning mode
3. Connect to AP with your phone or computer
4. Browser opens captive portal automatically
5. If not, navigate to `http://192.168.4.1`

### Configuration Interface
- **Network List**: Shows available WiFi networks with signal strength
- **Security**: Lock icon indicates password-protected networks
- **Refresh**: Click to rescan for networks
- **Reset**: Clear saved credentials

### Provisioning Mode Controls
- **Long Press (1-5s)**: Clear saved WiFi credentials and restart
- **Any button press**: Resets the 3-minute timeout
- Device will enter deep sleep after **3 minutes** of inactivity to save battery
- This is important for factory firmware - devices may sit in packaging for months!

### Reconfiguring WiFi
To change WiFi settings:
1. Long press the button during provisioning mode, OR
2. Call `ha.clearWiFiCredentials()` in code and reupload

## Developer Mode

Triple-click the button to enable Developer Mode:

- Extends sleep timeout to 3 minutes
- Allows time for firmware upload without device sleeping
- Visual indicator: Both LEDs blink 3 times + Random color RGB effect
- Triple-click again to disable

## Sleep Behavior

The device automatically enters deep sleep after a period of inactivity:

| Condition | Timeout | Notes |
|-----------|---------|-------|
| HA Connected | 10 seconds | Quick sleep for battery saving |
| HA Not Connected | 3 minutes | Longer timeout for connection attempts |
| Developer Mode | 3 minutes | Extended timeout for development |

**Important**: Every button action resets the sleep timer.

## Button Detection Timing

| Action | Press Duration | Release Gap |
|--------|---------------|-------------|
| Single Click | ≤ 1 second | No next press within 0.5s |
| Double Click | ≤ 1 second each | ≤ 1 second between clicks |
| Triple Click | ≤ 0.8 second each | ≤ 0.8 second between clicks |
| Long Press (Switch 3) | 1-5 seconds | N/A |
| Long Press (WiFi Reset) | ≥ 6 seconds | N/A |

> **Note**: The 6-second WiFi reset works **both when awake AND when waking from deep sleep**. Simply hold the button for 6+ seconds at any time to reset WiFi credentials and enter AP mode.

## Persistent Storage

The following data is saved to flash and survives deep sleep/reset:

- WiFi credentials (SSID and password) - if using provisioning
- Switch 1/2/3 states
- Last battery percentage (for anti-jump filter)

## Power Consumption

| Mode | Current | Notes |
|------|---------|-------|
| Active (WiFi) | ~80-150mA | Depends on WiFi activity |
| Deep Sleep | ~10µA | GPIO wake-up configured |

## Troubleshooting

### "text section exceeds available space in board" error
- This error means the code is too large for the default partition scheme
- **Solution**: Go to **Tools** → **Partition Scheme** → Select **"Huge APP (3MB No OTA/1MB SPIFFS)"**
- Alternative: Select **"Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)"**

### Device won't wake from deep sleep
- Ensure GPIO2 is properly connected to the button
- Check that the button pulls GPIO2 to GND when pressed

### WiFi connection fails
- Verify SSID and password are correct
- Check WiFi signal strength
- The red LED will blink continuously if connection fails

### Switches appear ON after reset
- This is fixed in the current version
- States are loaded from flash before switch creation

### Battery percentage jumps unexpectedly
- The anti-jump filter prevents increases < 5%
- For accurate readings, ensure the voltage divider is calibrated

## Serial Monitor Output

Connect via Serial Monitor (115200 baud) to see:
- Boot reason (fresh boot / deep sleep wake)
- WiFi connection status
- Button events detected
- Switch state changes
- Sleep timeout countdown
- Battery readings

## License

This example is part of the SeeedHADiscovery library.

## Support

For issues and feature requests, please visit the [GitHub repository](https://github.com/limengdu/SeeedHADiscovery).

