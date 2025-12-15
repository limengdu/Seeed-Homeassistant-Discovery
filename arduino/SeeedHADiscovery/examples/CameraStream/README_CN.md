# 摄像头推流示例

通过 MJPEG 将 XIAO ESP32-S3 Sense 摄像头视频流推送到 Home Assistant。支持自动发现实现无缝集成。

## 功能特性

- MJPEG 视频流
- 静态图片捕获
- 摄像头预览 Web UI
- Home Assistant 自动发现
- 双核处理（摄像头在核心 0，HA 在核心 1）
- 支持 PSRAM 以获得高质量图像

## 硬件要求

- **XIAO ESP32-S3 Sense** 带 OV2640 摄像头模块
- 必须启用 PSRAM

> ⚠️ 本示例仅适用于 XIAO ESP32-S3 Sense！

## 软件依赖

### 需要安装的库

通过 Arduino 库管理器安装：

| 库名称 | 作者 | 说明 |
|-------|------|------|
| **ArduinoJson** | Benoit Blanchon | JSON 解析 |
| **WebSockets** | Markus Sattler | WebSocket 通信 |

### 内置库

- **esp_camera**（ESP32 Arduino 核心）

### SeeedHADiscovery 库

从 [GitHub](https://github.com/limengdu/SeeedHADiscovery) 手动安装。

## Arduino IDE 设置

**重要**：上传前配置这些设置！

| 设置 | 值 |
|-----|---|
| 开发板 | XIAO_ESP32S3 |
| PSRAM | OPI PSRAM |
| Flash Size | 8MB |

## 快速开始

### 1. 配置 WiFi

```cpp
const char* WIFI_SSID = "你的WiFi名称";
const char* WIFI_PASSWORD = "你的WiFi密码";
```

### 2. 上传

1. 选择开发板：**XIAO_ESP32S3**
2. 启用 PSRAM：**工具** → **PSRAM** → **OPI PSRAM**
3. 上传程序

### 3. 访问摄像头

上传后，查看串口监视器获取 URL：

| URL | 说明 |
|-----|------|
| `http://<IP>:82/` | 带实时预览的 Web UI |
| `http://<IP>:82/camera` | 静态图片捕获 |
| `http://<IP>:82/stream` | MJPEG 视频流 |

## 摄像头配置

### 帧大小选项

```cpp
#define CAMERA_FRAME_SIZE FRAMESIZE_VGA  // 640x480（推荐）
```

可用大小：
- `FRAMESIZE_QQVGA` (160x120)
- `FRAMESIZE_QVGA` (320x240)
- `FRAMESIZE_VGA` (640x480) ← 默认
- `FRAMESIZE_SVGA` (800x600)
- `FRAMESIZE_XGA` (1024x768)

### JPEG 质量

```cpp
#define CAMERA_JPEG_QUALITY 12  // 0-63，越低质量越好
```

## Home Assistant 集成

### 添加摄像头到 HA

1. 通过 **Seeed HA Discovery** 集成添加设备
2. 将出现一个 "Camera Status" 传感器
3. 手动添加 MJPEG 摄像头：

```yaml
# configuration.yaml
camera:
  - platform: mjpeg
    name: "XIAO Camera"
    mjpeg_url: http://<设备IP>:82/stream
    still_image_url: http://<设备IP>:82/camera
```

## 架构

```
核心 0：摄像头服务器（端口 82）
  └── MJPEG 流
  └── 静态图片捕获

核心 1：主循环
  └── SeeedHADiscovery（端口 80）
  └── Home Assistant 通信
```

## 引脚配置

| 功能 | GPIO |
|-----|------|
| XCLK | 10 |
| SIOD (SDA) | 40 |
| SIOC (SCL) | 39 |
| Y2-Y9 | 15,17,18,16,14,12,11,48 |
| VSYNC | 38 |
| HREF | 47 |
| PCLK | 13 |

## 故障排除

### 摄像头初始化失败
- 在 Arduino IDE 中启用 PSRAM
- 检查摄像头模块连接
- 确认使用的是 XIAO ESP32-S3 Sense

### 视频流慢/卡顿
- 减小帧大小
- 增加 JPEG 质量数值（降低质量）
- 检查 WiFi 信号强度

### 未找到 PSRAM
- 进入 **工具** → **PSRAM** → **OPI PSRAM**
- 重新上传程序

## 许可证

SeeedHADiscovery 库的一部分。

