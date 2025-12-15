# 按钮 BLE 示例

通过蓝牙低功耗向 Home Assistant 发送按钮事件。支持单击、双击、三击和长按检测。

## 功能特性

- 多种按钮事件类型（单击、双击、三击、长按）
- 三个独立开关对应不同按键类型
- BTHome v2 协议自动发现
- GATT 双向通信
- 按键时立即广播状态
- 超低功耗设计

## 硬件要求

- XIAO ESP32-C3/C6/S3 或 XIAO nRF52840
- 按钮连接到 GPIO（默认 D1）

### 按钮接线

| 按钮引脚 | 连接 |
|---------|------|
| 一端 | GPIO D1（默认） |
| 另一端 | GND |

已启用内部上拉电阻。

## 软件依赖

### ESP32

通过 Arduino 库管理器安装：

| 库名称 | 说明 |
|-------|------|
| **NimBLE-Arduino** | ESP32 BLE 协议栈 |

### nRF52840

- **ArduinoBLE** (mbed) 或 **Bluefruit** (Adafruit) - 已内置

### SeeedHADiscoveryBLE 库

从 [GitHub](https://github.com/limengdu/SeeedHADiscovery) 手动安装。

## 快速开始

### 1. 配置设备名称

```cpp
const char* DEVICE_NAME = "XIAO Button";
```

### 2. 配置按钮引脚

```cpp
#define BUTTON_PIN D1
```

### 3. 上传并使用

1. 上传程序
2. Home Assistant 通过 BTHome 自动发现
3. 按下按钮切换开关

## 按钮事件

| 事件 | 动作 | 时序 |
|-----|------|------|
| 单击 | 切换单击开关 | 按下 < 1秒，无后续 |
| 双击 | 切换双击开关 | 300ms 内按 2 次 |
| 三击 | 切换双击开关 | 300ms 内按 3 次 |
| 长按 | 切换长按开关 | 按住 > 1 秒 |

## 创建的实体

| 实体 | 类型 | 触发方式 |
|-----|------|---------|
| Single Click | 开关 | 单次按下 |
| Double Click | 开关 | 双击/三击 |
| Long Press | 开关 | 长按 > 1秒 |

## 配置选项

| 选项 | 默认值 | 说明 |
|-----|-------|------|
| `DEVICE_NAME` | "XIAO Button" | BLE 设备名称 |
| `BUTTON_PIN` | D1 | 按钮 GPIO 引脚 |
| `LONG_PRESS_TIME` | 1000ms | 长按阈值 |
| `DOUBLE_CLICK_TIME` | 300ms | 双击间隔 |
| `ADVERTISE_INTERVAL` | 5000ms | BTHome 广播间隔 |

## 工作原理

1. 检测按钮按下 → 分析按键模式
2. 确定事件类型（单击/双击/三击/长按）
3. 切换对应开关状态
4. 立即通过 BTHome 广播状态
5. HA 接收并更新实体状态

## 状态同步

- 本地按键 → 更新开关 → 广播到 HA
- HA 远程控制 → 更新开关 → 同步传感器状态

## 测试

1. 打开串口监视器（115200 波特率）
2. 按下按钮并观察：
   - 检测到的事件类型
   - 开关状态变化
   - BLE 广播发送

## 故障排除

### 按钮无响应
- 检查接线（按钮接 GND）
- 验证 BUTTON_PIN 设置
- 查看串口监视器的事件

### 设备未被发现
- 确保 HA 有 BLE 适配器或代理
- 检查设备是否在广播

### 双击无法识别
- 按得更快（300ms 内）
- 如需要可调整 `DOUBLE_CLICK_TIME`

## 许可证

SeeedHADiscoveryBLE 库的一部分。

