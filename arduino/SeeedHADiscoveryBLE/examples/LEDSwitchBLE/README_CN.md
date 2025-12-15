# LED 开关 BLE 示例

通过蓝牙低功耗从 Home Assistant 控制 LED。支持双向 GATT 通信和 BTHome 协议。

## 功能特性

- 通过 BLE 从 Home Assistant 控制 LED 开关
- 双向 GATT 通信
- BTHome v2 协议自动发现
- LED 状态传感器用于状态反馈
- 超低功耗运行

## 硬件要求

- XIAO ESP32-C3/C6/S3 或 XIAO nRF52840
- LED（板载或外接）

### LED 配置

| 开发板 | 板载 LED | 说明 |
|-------|---------|------|
| XIAO ESP32-S3 | GPIO21 | 有用户 LED |
| XIAO ESP32-C6 | GPIO15 | 有用户 LED |
| XIAO ESP32-C3 | 无 | **没有用户 LED** - 需要外接 |
| XIAO nRF52840 | 内置 | 有用户 LED |

## 软件依赖

### ESP32

通过 Arduino 库管理器安装：

| 库名称 | 说明 |
|-------|------|
| **NimBLE-Arduino** | ESP32 BLE 协议栈 |

### nRF52840 (mbed)

- **ArduinoBLE**（已内置）

### SeeedHADiscoveryBLE 库

从 [GitHub](https://github.com/limengdu/SeeedHADiscovery) 手动安装。

## 快速开始

### 1. 配置设备名称

```cpp
const char* DEVICE_NAME = "XIAO LED Controller";
```

### 2. 配置外接 LED（如果需要）

对于 XIAO ESP32-C3：
```cpp
#define EXTERNAL_LED
#define LED_PIN D0
```

### 3. 上传并发现

1. 上传程序
2. Home Assistant 将通过 BTHome 自动发现
3. 设备在 HA 中显示为 BLE 设备

## 工作原理

```
Home Assistant ──BLE GATT──> 设备
     │                         │
     │    写入命令              │
     ▼                         ▼
  [开关]                 [LED 控制]
     │                         │
     │    通知状态              │
     ◄─────────────────────────┘
```

1. 设备广播 BTHome 数据
2. HA 发现并通过 GATT 连接
3. HA 写入命令到控制特征值
4. 设备控制 LED 并通知状态变化

## GATT 服务

| UUID | 说明 |
|------|------|
| 控制服务 | `SEEED_CONTROL_SERVICE_UUID` |
| 命令特征值 | `SEEED_CONTROL_COMMAND_CHAR_UUID` |
| 状态特征值 | `SEEED_CONTROL_STATE_CHAR_UUID` |

### 命令格式

```
[开关索引][状态]
例如：0x00 0x01 = 开关 0 开启
      0x00 0x00 = 开关 0 关闭
```

## 配置选项

| 选项 | 默认值 | 说明 |
|-----|-------|------|
| `DEVICE_NAME` | "XIAO LED Controller" | BLE 设备名称 |
| `ADVERTISE_INTERVAL` | 5000ms | BTHome 广播间隔 |
| `LED_ACTIVE_LOW` | true | LED 极性 |

## 使用 nRF Connect 测试

1. 扫描并连接设备
2. 找到控制服务
3. 写入命令特征值：
   - `00 01` 开启
   - `00 00` 关闭

## 故障排除

### 设备未被发现
- 确保 HA 主机启用了 BLE
- 或使用 ESPHome 蓝牙代理
- 检查设备是否在广播（串口监视器）

### LED 无响应
- 检查 LED_ACTIVE_LOW 设置
- 如果是外接 LED，验证接线

## 许可证

SeeedHADiscoveryBLE 库的一部分。

