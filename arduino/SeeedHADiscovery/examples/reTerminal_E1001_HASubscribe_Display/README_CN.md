# reTerminal E1001 HA 显示仪表板

在 reTerminal E1001 的四阶灰度墨水屏上显示 Home Assistant 实体状态。创建美观的仪表板并自动更新。

## 功能特性

- 订阅任意 Home Assistant 实体
- 四阶灰度墨水屏（800x480）
- 简洁的单色仪表板界面
- 最多 6 个实体卡片
- 自动显示刷新
- 连接状态指示
- 智能刷新逻辑（避免不必要的更新）

## 硬件要求

- **reTerminal E1001** 带四阶灰度墨水屏
- 显示分辨率：800x480

## 支持的颜色

仅支持 4 阶灰度：

| 颜色常量 | 说明 |
|---------|------|
| `TFT_GRAY_0` | 黑色 |
| `TFT_GRAY_1` | 深灰 |
| `TFT_GRAY_2` | 浅灰 |
| `TFT_GRAY_3` | 白色 |

## 软件依赖

### 需要安装的库

| 库名称 | 来源 | 说明 |
|-------|------|------|
| **Seeed_GFX** | [GitHub](https://github.com/Seeed-Studio/Seeed_GFX) | 墨水屏图形库 |
| **ArduinoJson** | 库管理器 | JSON 解析 |
| **WebSockets** | 库管理器 | WebSocket 通信 |

### SeeedHADiscovery 库

从 [GitHub](https://github.com/limengdu/SeeedHADiscovery) 手动安装。

## 快速开始

### 1. 配置 WiFi

```cpp
const char* WIFI_SSID = "你的WiFi名称";
const char* WIFI_PASSWORD = "你的WiFi密码";
```

### 2. 上传

1. 选择 reTerminal E1001 对应的开发板
2. 确保在 User_Setup.h 中定义了 `EPAPER_ENABLE`
3. 上传程序

### 3. 在 Home Assistant 中配置

1. 在 **设置** → **设备与服务** 中找到设备
2. 点击 **配置**
3. 选择要订阅的实体（最多 6 个）
4. 保存 - 显示屏将自动更新

## 显示布局

```
┌─────────────────────────────────────────────────────┐
│  Home Assistant Dashboard              [状态]       │
├───────────────┬───────────────┬───────────────────┬─┤
│   卡片 1      │   卡片 2      │   卡片 3          │ │
│   [数值]      │   [数值]      │   [数值]          │ │
├───────────────┼───────────────┼───────────────────┼─┤
│   卡片 4      │   卡片 5      │   卡片 6          │ │
│   [数值]      │   [数值]      │   [空]            │ │
├───────────────┴───────────────┴───────────────────┴─┤
│  设备信息 | IP 地址 | 运行时间 | 实体数量            │
└─────────────────────────────────────────────────────┘
```

## 刷新逻辑

| 触发条件 | 说明 |
|---------|------|
| 初始刷新 | 首次数据收集后（等待 5 秒） |
| 配置变更 | 在 HA 中添加/删除实体时 |
| 定时刷新 | 每 5 分钟 |
| 连接变化 | HA 上线/掉线时 |

## 配置选项

| 选项 | 默认值 | 说明 |
|-----|-------|------|
| `DISPLAY_REFRESH_INTERVAL` | 300000ms | 定时刷新间隔（5分钟） |
| `DATA_COLLECTION_WAIT` | 5000ms | 初始刷新前等待时间 |
| `MAX_DISPLAY_ENTITIES` | 6 | 最大显示实体数 |

## 串口输出

通过 Serial1 输出调试信息（引脚 43/44）：
- 连接状态
- 实体更新
- 刷新触发

## 设备类别图标

| 设备类别 | 显示 |
|---------|------|
| temperature | TEMP |
| humidity | HUM |
| battery | BAT |
| illuminance | LUX |
| power | PWR |
| energy | NRG |
| pressure | HPA |
| voltage | V |
| current | A |

## 故障排除

### 显示不更新
- 检查是否定义了 `EPAPER_ENABLE`
- 验证在 HA 中订阅了实体
- 检查 Serial1 输出的错误信息

### "EPAPER_ENABLE not defined"
- 在 TFT_eSPI/Seeed_GFX 的 User_Setup.h 中启用墨水屏

### 刷新慢
- 墨水屏刷新需要几秒钟
- 这是墨水屏的正常行为

## 许可证

SeeedHADiscovery 库的一部分。

