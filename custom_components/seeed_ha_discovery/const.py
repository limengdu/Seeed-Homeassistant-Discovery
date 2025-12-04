"""
Seeed HA Discovery - 常量定义文件
Constants for the Seeed HA Discovery integration.

这个文件定义了集成中使用的所有常量，包括：
- 域名和制造商信息
- 配置相关的键名
- 默认端口设置
- mDNS 服务类型
- WebSocket 消息类型
"""
from typing import Final

# =============================================================================
# 基本信息 | Basic Information
# =============================================================================

# 集成域名，Home Assistant 用这个来识别集成
# Integration domain, used by Home Assistant to identify the integration
DOMAIN: Final = "seeed_ha_discovery"

# 制造商名称，显示在设备信息中
# Manufacturer name, shown in device info
MANUFACTURER: Final = "Seeed Studio"

# =============================================================================
# 配置键名 | Configuration Keys
# =============================================================================

# 设备唯一标识符
# Unique device identifier
CONF_DEVICE_ID: Final = "device_id"

# 设备主机地址（IP 或主机名）
# Device host address (IP or hostname)
CONF_HOST: Final = "host"

# WebSocket 端口
# WebSocket port
CONF_PORT: Final = "port"

# 设备型号
# Device model
CONF_MODEL: Final = "model"

# =============================================================================
# 默认值 | Default Values
# =============================================================================

# HTTP 服务器默认端口
# Default HTTP server port
DEFAULT_HTTP_PORT: Final = 80

# WebSocket 服务器默认端口
# Default WebSocket server port
DEFAULT_WS_PORT: Final = 81

# 重连间隔（秒）
# Reconnect interval in seconds
RECONNECT_INTERVAL: Final = 30

# 心跳间隔（秒）
# Heartbeat interval in seconds
HEARTBEAT_INTERVAL: Final = 15

# =============================================================================
# mDNS 配置 | mDNS Configuration
# =============================================================================

# mDNS 服务类型，用于自动发现设备
# mDNS service type for auto-discovery
# 设备会广播这个服务类型，HA 会自动发现
ZEROCONF_SERVICE_TYPE: Final = "_seeed_ha._tcp.local."

# =============================================================================
# WebSocket 消息类型 | WebSocket Message Types
# =============================================================================

# 心跳检测 - 设备发送
# Heartbeat ping - sent by device
MSG_TYPE_PING: Final = "ping"

# 心跳响应 - HA 回复
# Heartbeat response - sent by HA
MSG_TYPE_PONG: Final = "pong"

# 状态更新 - 设备上报传感器数据
# State update - device reports sensor data
MSG_TYPE_STATE: Final = "state"

# 设备发现 - 设备上报自己支持的实体
# Device discovery - device reports its entities
MSG_TYPE_DISCOVERY: Final = "discovery"

# =============================================================================
# 支持的平台 | Supported Platforms
# =============================================================================

# 当前只支持传感器平台
# Currently only sensor platform is supported
PLATFORMS: Final = [
    "sensor",
]
