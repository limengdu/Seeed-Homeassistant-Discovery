# Camera Stream Example

Stream video from XIAO ESP32-S3 Sense camera to Home Assistant via MJPEG. Auto-discovery enabled for seamless integration.

## Features

- MJPEG video streaming
- Still image capture
- Web UI for camera preview
- Home Assistant auto-discovery
- Dual-core processing (camera on Core 0, HA on Core 1)
- PSRAM support for high quality images

## Hardware Requirements

- **XIAO ESP32-S3 Sense** with OV2640 camera module
- PSRAM must be enabled

> ⚠️ This example only works with XIAO ESP32-S3 Sense!

## Software Dependencies

### Required Libraries

Install via Arduino Library Manager:

| Library | Author | Description |
|---------|--------|-------------|
| **ArduinoJson** | Benoit Blanchon | JSON parsing |
| **WebSockets** | Markus Sattler | WebSocket communication |

### Built-in Libraries

- **esp_camera** (ESP32 Arduino Core)

### SeeedHADiscovery Library

Install manually from [GitHub](https://github.com/limengdu/SeeedHADiscovery).

## Arduino IDE Settings

**IMPORTANT**: Configure these settings before uploading!

| Setting | Value |
|---------|-------|
| Board | XIAO_ESP32S3 |
| PSRAM | OPI PSRAM |
| Flash Size | 8MB |

## Quick Start

### 1. Configure WiFi

```cpp
const char* WIFI_SSID = "Your_WiFi_SSID";
const char* WIFI_PASSWORD = "Your_WiFi_Password";
```

### 2. Upload

1. Select board: **XIAO_ESP32S3**
2. Enable PSRAM: **Tools** → **PSRAM** → **OPI PSRAM**
3. Upload the sketch

### 3. Access Camera

After upload, check Serial Monitor for URLs:

| URL | Description |
|-----|-------------|
| `http://<IP>:82/` | Web UI with live preview |
| `http://<IP>:82/camera` | Still image capture |
| `http://<IP>:82/stream` | MJPEG video stream |

## Camera Configuration

### Frame Size Options

```cpp
#define CAMERA_FRAME_SIZE FRAMESIZE_VGA  // 640x480 (recommended)
```

Available sizes:
- `FRAMESIZE_QQVGA` (160x120)
- `FRAMESIZE_QVGA` (320x240)
- `FRAMESIZE_VGA` (640x480) ← Default
- `FRAMESIZE_SVGA` (800x600)
- `FRAMESIZE_XGA` (1024x768)

### JPEG Quality

```cpp
#define CAMERA_JPEG_QUALITY 12  // 0-63, lower = better quality
```

## Home Assistant Integration

### Add Camera to HA

1. Add device via **Seeed HA Discovery** integration
2. A "Camera Status" sensor will appear
3. Add MJPEG camera manually:

```yaml
# configuration.yaml
camera:
  - platform: mjpeg
    name: "XIAO Camera"
    mjpeg_url: http://<device_ip>:82/stream
    still_image_url: http://<device_ip>:82/camera
```

## Architecture

```
Core 0: Camera Server (port 82)
  └── MJPEG streaming
  └── Still image capture

Core 1: Main Loop
  └── SeeedHADiscovery (port 80)
  └── Home Assistant communication
```

## Pin Configuration

| Function | GPIO |
|----------|------|
| XCLK | 10 |
| SIOD (SDA) | 40 |
| SIOC (SCL) | 39 |
| Y2-Y9 | 15,17,18,16,14,12,11,48 |
| VSYNC | 38 |
| HREF | 47 |
| PCLK | 13 |

## Troubleshooting

### Camera init failed
- Enable PSRAM in Arduino IDE
- Check camera module connection
- Verify using XIAO ESP32-S3 Sense

### Stream is slow/choppy
- Reduce frame size
- Increase JPEG quality number (lower quality)
- Check WiFi signal strength

### PSRAM not found
- Go to **Tools** → **PSRAM** → **OPI PSRAM**
- Re-upload the sketch

## License

Part of the SeeedHADiscovery library.

