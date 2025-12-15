# HA 状态订阅 BLE 示例

通过蓝牙低功耗接收 Home Assistant 实体状态。完全动态 - 无需硬编码！

## 功能特性

- 订阅任意 Home Assistant 实体
- 通过 HA 界面完全动态配置
- 通过 BLE 实时状态更新
- 最多支持 16 个实体
- 自动显示实体名称
- GATT 双向通信

## 硬件要求

- XIAO ESP32-C3/C6/S3 或 XIAO nRF52840

## 软件依赖

### ESP32

通过 Arduino 库管理器安装：

| 库名称 | 说明 |
|-------|------|
| **NimBLE-Arduino** | ESP32 BLE 协议栈 |

### nRF52840

- **ArduinoBLE** (mbed) - 已内置

### SeeedHADiscoveryBLE 库

从 [GitHub](https://github.com/limengdu/SeeedHADiscovery) 手动安装。

## 快速开始

### 1. 上传代码

直接上传 - 代码中无需配置！

```cpp
const char* DEVICE_NAME = "XIAO HA State Monitor";
```

### 2. 在 Home Assistant 中配置

1. 在 HA 中找到 BLE 设备
2. 在设备选项中点击 **配置**
3. 选择你想接收的任意实体
4. 保存 - 它们会自动出现！

### 3. 查看状态

状态显示在串口监视器中，可在代码中使用。

## 工作原理

```
┌─────────────────┐         ┌─────────────────┐
│  Home Assistant │         │      设备       │
│                 │         │                 │
│    实体状态     │──BLE───▶│  onHAState()    │
│                 │         │     回调        │
│      配置       │◀──BLE───│                 │
└─────────────────┘         └─────────────────┘
```

1. 上传代码到设备
2. 在 HA 中配置订阅
3. HA 通过 BLE GATT 推送实体状态
4. 设备接收并处理状态

## 回调函数

```cpp
void onHAStateReceived(uint8_t entityIndex, 
                       const char* entityId, 
                       const char* state, 
                       float numericValue) {
    Serial.print("实体: ");
    Serial.println(entityId);
    Serial.print("状态: ");
    Serial.println(state);
    Serial.print("数值: ");
    Serial.println(numericValue);
}
```

## 访问状态

```cpp
// 通过索引获取状态
SeeedBLEHAState* state = ble.getHAState(0);
if (state && state->hasValue()) {
    String entityId = state->getEntityId();
    String stateStr = state->getString();
    float value = state->getFloat();
}

// 获取订阅的实体数量
uint8_t count = ble.getSubscribedEntityCount();
```

## 配置选项

| 选项 | 默认值 | 说明 |
|-----|-------|------|
| `DEVICE_NAME` | "XIAO HA State Monitor" | BLE 设备名称 |
| `ADVERTISE_INTERVAL` | 5000ms | BTHome 广播间隔 |
| `MAX_ENTITIES` | 16 | 最大订阅实体数 |

## 使用场景

- 在屏幕上显示传感器值
- 响应实体变化
- 创建便携式仪表板
- 构建自动化触发器

## 示例：温度警报

```cpp
void onHAStateReceived(...) {
    if (strstr(entityId, "temperature") != NULL) {
        if (numericValue > 30.0) {
            Serial.println("温度过高！");
            // 触发警报、LED 等
        }
    }
}
```

## 示例：灯光状态

```cpp
void onHAStateReceived(...) {
    if (strstr(entityId, "light.") != NULL) {
        bool isOn = (strcmp(state, "on") == 0);
        Serial.print("灯光: ");
        Serial.println(isOn ? "开启" : "关闭");
    }
}
```

## 串口输出格式

```
╔══════════════════════════════════════════╗
║       收到 HA 状态更新                    ║
╠══════════════════════════════════════════╣
║ 索引:  0                                 ║
║ 实体: sensor.temperature                 ║
║ 状态:  25.5                              ║
║ 数值:  25.50                             ║
╚══════════════════════════════════════════╝
```

## 故障排除

### 没有实体出现
- 在 HA 设备设置中配置订阅
- 检查 BLE 连接状态

### 状态不更新
- 验证 BLE 连接
- 检查 HA 集成状态

### 设备未被发现
- 确保 HA 有 BLE 适配器或代理
- 检查设备是否在广播

## 许可证

SeeedHADiscoveryBLE 库的一部分。

