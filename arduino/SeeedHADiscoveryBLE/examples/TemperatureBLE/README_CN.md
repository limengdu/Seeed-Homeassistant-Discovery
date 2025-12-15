# 温湿度 BLE 示例

使用 BTHome v2 协议通过蓝牙低功耗向 Home Assistant 广播温湿度数据。

## 功能特性

- 温度传感器广播
- 湿度传感器广播
- BTHome v2 协议自动发现
- 超低功耗运行
- 可选 DHT22 传感器支持
- 模拟数据模式用于测试

## 硬件要求

- XIAO ESP32-C3/C6/S3 或 XIAO nRF52840
- DHT22 传感器（可选 - 可使用模拟数据）

### DHT22 接线

| DHT22 引脚 | 连接 |
|-----------|------|
| VCC | 3.3V |
| GND | GND |
| DATA | D2（可配置） |

## 软件依赖

### ESP32

通过 Arduino 库管理器安装：

| 库名称 | 说明 |
|-------|------|
| **NimBLE-Arduino** | ESP32 BLE 协议栈 |
| **DHT sensor library** | DHT22 支持（如果使用） |

### nRF52840

- **ArduinoBLE** (mbed) 或 **Bluefruit** (Adafruit) - 已内置
- **DHT sensor library**（如果使用 DHT22）

### SeeedHADiscoveryBLE 库

从 [GitHub](https://github.com/limengdu/SeeedHADiscovery) 手动安装。

## 快速开始

### 1. 配置设备名称

```cpp
const char* DEVICE_NAME = "XIAO Temp/Humidity";
```

### 2. 启用 DHT22（可选）

使用真实传感器：
```cpp
#include <DHT.h>
#define USE_DHT_SENSOR
```

### 3. 上传

1. 上传程序
2. Home Assistant 通过 BTHome 自动发现
3. 温湿度传感器出现在 HA 中

## 配置选项

| 选项 | 默认值 | 说明 |
|-----|-------|------|
| `DEVICE_NAME` | "XIAO Temp/Humidity" | BLE 设备名称 |
| `ADVERTISE_INTERVAL` | 10000ms | 广播间隔（10秒） |
| `DHT_PIN` | D2 | DHT22 数据引脚 |
| `DHT_TYPE` | DHT22 | 传感器类型 |

## 模拟数据模式

当未定义 `USE_DHT_SENSOR` 时：
- 温度：在 20-30°C 之间波动
- 湿度：在 40-70% 之间波动

非常适合没有硬件的测试！

## BTHome 协议

使用 BTHome v2 格式广播数据：
- 被 Home Assistant 自动发现
- 无需配对
- 支持 ESPHome 蓝牙代理

## 功耗优化

| 参数 | 效果 |
|-----|------|
| 更长间隔 | 更低功耗 |
| 更短间隔 | 更快更新 |

推荐：电池供电时使用 10-60 秒间隔。

## 创建的实体

| 实体 | 类型 | 单位 |
|-----|------|------|
| Temperature | 传感器 | °C |
| Humidity | 传感器 | % |

## Home Assistant 要求

- HA 主机上的蓝牙适配器，或
- ESPHome 蓝牙代理
- 启用 BTHome 集成

## 测试

1. 打开串口监视器（115200 波特率）
2. 观察广播消息：
   ```
   Broadcast: Temp=25.0C, Humidity=55%
   ```
3. 在 HA 中检查新的 BTHome 设备

## 故障排除

### 设备未被发现
- 确保 HA 有 BLE 功能
- 检查设备是否在广播
- 发现可能需要几分钟

### DHT22 读取失败
- 检查接线连接
- 验证 3.3V 供电
- 确保正确的 DHT_TYPE 设置

### 数值不更新
- 检查 ADVERTISE_INTERVAL
- 验证 HA 中的 BLE 连接

## 许可证

SeeedHADiscoveryBLE 库的一部分。

