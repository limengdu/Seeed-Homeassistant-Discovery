# Seeed Home Assistant Discovery (WiFi 版)

[![Version](https://img.shields.io/badge/版本-1.5.1-blue.svg)](https://github.com/limengdu/Seeed-Homeassistant-Discovery)
[![License](https://img.shields.io/badge/许可证-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/平台-ESP32-orange.svg)](https://www.espressif.com/)

一个轻量级的 Arduino 库，用于通过 WiFi 将 ESP32 设备连接到 Home Assistant。支持自动设备发现、实时通信以及丰富的传感器/开关功能。

## ✨ 功能特性

- **自动发现** - 通过 mDNS 自动发现设备，无需手动配置
- **实时通信** - 基于 WebSocket 的双向实时通信
- **传感器支持** - 向 HA 报告传感器数据（温度、湿度、电量等）
- **开关控制** - 接收来自 HA 的控制命令，控制 LED、继电器等
- **HA 状态订阅** - 订阅并接收 Home Assistant 实体状态变化
- **摄像头串流** - 支持 MJPEG 摄像头串流（ESP32-S3 Sense）
- **WiFi 配网** - 基于 Web 的强制门户配网
- **5GHz WiFi** - ESP32-C5 支持双频 2.4GHz/5GHz WiFi

## 🔧 支持的硬件

| 开发板 | WiFi 频段 | 摄像头 | 备注 |
|--------|----------|--------|------|
| XIAO ESP32-C3 | 2.4GHz | ❌ | 低功耗、紧凑型 |
| XIAO ESP32-C5 | 2.4GHz + 5GHz | ❌ | 支持双频 WiFi |
| XIAO ESP32-C6 | 2.4GHz | ❌ | 支持 Thread/Zigbee |
| XIAO ESP32-S3 | 2.4GHz | ✅ | 支持摄像头 + PSRAM |
| XIAO ESP32-S3 Sense | 2.4GHz | ✅ | 内置 OV2640 摄像头 |

## 📦 安装

### 方法 1：下载 ZIP

1. 前往 [GitHub 仓库](https://github.com/limengdu/Seeed-Homeassistant-Discovery)
2. 点击 **Code → Download ZIP**
3. 解压 ZIP 文件
4. 将 `arduino/SeeedHADiscovery` 文件夹复制到 Arduino 库目录：
   - Windows: `文档/Arduino/libraries/`
   - macOS: `~/Documents/Arduino/libraries/`
   - Linux: `~/Arduino/libraries/`
5. 重启 Arduino IDE

### 方法 2：Git 克隆

```bash
git clone https://github.com/limengdu/Seeed-Homeassistant-Discovery.git
```

然后将 `arduino/SeeedHADiscovery` 文件夹复制到 Arduino 库目录。

> **注意：** 此库不在 Arduino 库管理器中，请使用手动安装方式。

## 📚 依赖库

| 库名 | 版本 | 用途 |
|------|------|------|
| [ArduinoJson](https://github.com/bblanchon/ArduinoJson) | ^7.0.0 | JSON 序列化 |
| [WebSockets](https://github.com/Links2004/arduinoWebSockets) | ^2.4.0 | WebSocket 通信 |

## 🚀 快速开始

### 基础传感器示例

```cpp
#include <SeeedHADiscovery.h>

SeeedHADiscovery ha;
SeeedHASensor* tempSensor;
SeeedHASensor* humiSensor;

void setup() {
    Serial.begin(115200);
    
    // 设置设备信息（可选）
    ha.setDeviceInfo("客厅传感器", "XIAO ESP32-C3", "1.0.0");
    
    // 连接 WiFi
    ha.begin("你的WiFi名称", "你的WiFi密码");
    
    // 添加传感器
    tempSensor = ha.addSensor("temperature", "温度", "temperature", "°C");
    humiSensor = ha.addSensor("humidity", "湿度", "humidity", "%");
}

void loop() {
    ha.handle();  // 必须在 loop 中调用！
    
    // 更新传感器值
    tempSensor->setValue(25.5);
    humiSensor->setValue(60.0);
    
    delay(5000);
}
```

### 开关控制示例

```cpp
#include <SeeedHADiscovery.h>

SeeedHADiscovery ha;
SeeedHASwitch* ledSwitch;

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    
    ha.begin("你的WiFi名称", "你的WiFi密码");
    
    // 添加开关并设置回调
    ledSwitch = ha.addSwitch("led", "LED 灯", "mdi:led-on");
    ledSwitch->onStateChange([](bool state) {
        digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
        Serial.printf("LED: %s\n", state ? "开启" : "关闭");
    });
}

void loop() {
    ha.handle();
}
```

### WiFi 配网示例

```cpp
#include <SeeedHADiscovery.h>

SeeedHADiscovery ha;
bool wifiConnected = false;

void setup() {
    Serial.begin(115200);
    
    // 启动带配网支持的模式
    // 如果没有保存的凭据，将启动 AP 模式进行配置
    wifiConnected = ha.beginWithProvisioning("My_IoT_Device_AP");
    
    if (wifiConnected) {
        // 在这里添加传感器/开关
    }
}

void loop() {
    ha.handle();  // 同时处理 HA 通信和配网
}
```

## 📂 示例程序

| 示例 | 描述 |
|------|------|
| [TemperatureHumidity](examples/TemperatureHumidity/) | 基础传感器数据上报 |
| [LEDSwitch](examples/LEDSwitch/) | 可控制的 LED 开关 |
| [ButtonSwitch](examples/ButtonSwitch/) | 物理按钮 + HA 开关 |
| [HAStateSubscribe](examples/HAStateSubscribe/) | 订阅 HA 实体状态 |
| [CameraStream](examples/CameraStream/) | MJPEG 摄像头串流（S3 Sense） |
| [WiFiProvisioning](examples/WiFiProvisioning/) | 基于 Web 的 WiFi 配置 |
| [IoTButtonV2_DeepSleep](examples/IoTButtonV2_DeepSleep/) | 带深度睡眠的电池供电 IoT 按钮 |
| [reTerminal_E1001_HASubscribe_Display](examples/reTerminal_E1001_HASubscribe_Display/) | E-Paper 显示屏显示 HA 状态 |
| [reTerminal_E1002_HASubscribe_Display](examples/reTerminal_E1002_HASubscribe_Display/) | 彩色 E-Paper 显示屏显示 HA 状态 |

## 🔌 API 参考

### SeeedHADiscovery 类

#### 配置
```cpp
void setDeviceInfo(const String& name, const String& model, const String& version);
void enableDebug(bool enable = true);
```

#### 连接
```cpp
bool begin(const char* ssid, const char* password);
bool beginWithProvisioning(const String& apSSID = "Seeed_IoT_Device_AP");
void clearWiFiCredentials();
void enableResetButton(int pin, bool activeLow = true);
```

#### 实体管理
```cpp
SeeedHASensor* addSensor(const String& id, const String& name, const String& deviceClass = "", const String& unit = "");
SeeedHASwitch* addSwitch(const String& id, const String& name, const String& icon = "");
```

#### HA 状态订阅
```cpp
void onHAState(HAStateCallback callback);
SeeedHAState* getHAState(const String& entityId);
```

#### 运行时
```cpp
void handle();  // 必须在 loop() 中调用
bool isWiFiConnected() const;
bool isHAConnected() const;
void notifySleep();  // 进入深度睡眠前调用
```

### SeeedHASensor 类

```cpp
void setValue(float value);
void setStateClass(const String& stateClass);
void setPrecision(int precision);
void setIcon(const String& icon);
```

### SeeedHASwitch 类

```cpp
void setState(bool state);
void toggle();
bool getState() const;
void onStateChange(SwitchCallback callback);
```

## 🔗 ESP32-C5 5GHz WiFi 支持

ESP32-C5 是唯一支持 5GHz WiFi 的 XIAO 开发板。配置 WiFi 频段模式：

```cpp
#include <WiFi.h>

void setup() {
    // 设置 WiFi 频段模式（需要 Arduino ESP32 Core 3.3.0+）
    #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 2)
        // WIFI_BAND_MODE_AUTO - 自动选择（默认）
        // WIFI_BAND_MODE_2G_ONLY - 仅 2.4GHz
        // WIFI_BAND_MODE_5G_ONLY - 仅 5GHz
        WiFi.setBandMode(WIFI_BAND_MODE_AUTO);
    #endif
    
    // 然后正常连接
    ha.begin("你的5GHz_WiFi名称", "你的密码");
}
```

## 🏠 Home Assistant 集成

本库需要配合 Home Assistant 的 [Seeed HA Discovery](https://github.com/limengdu/Seeed-Homeassistant-Discovery) 自定义集成使用。

### 安装步骤

1. 在 Home Assistant 中安装 HACS
2. 添加自定义仓库：`https://github.com/limengdu/Seeed-Homeassistant-Discovery`
3. 安装 "Seeed HA Discovery" 集成
4. 重启 Home Assistant
5. 进入 设置 → 设备与服务 → 添加集成 → Seeed HA Discovery

设备连接到同一网络后将自动被发现。

## 📝 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件。

## 🤝 贡献

欢迎贡献！请随时提交 Pull Request。

## 📧 支持

- GitHub Issues：[报告问题](https://github.com/limengdu/Seeed-Homeassistant-Discovery/issues)
- Seeed 论坛：[社区支持](https://forum.seeedstudio.com/)

