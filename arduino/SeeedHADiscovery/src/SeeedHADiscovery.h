/**
 * ============================================================================
 * Seeed Home Assistant Discovery - ESP32 连接 Home Assistant 的 Arduino 库
 * Seeed Home Assistant Discovery - Arduino library for connecting ESP32 to Home Assistant
 * ============================================================================
 *
 * 这个库让 ESP32 设备能够轻松连接到 Home Assistant：
 * - 支持 ESP32-C3、C6、S3 等型号
 * - 通过 mDNS 自动被 Home Assistant 发现
 * - 使用 WebSocket 实时推送传感器数据
 * - 简单易用的 API
 *
 * 使用方法：
 * 1. 创建 SeeedHADiscovery 实例
 * 2. 调用 begin() 连接 WiFi
 * 3. 使用 addSensor() 添加传感器
 * 4. 在 loop() 中调用 handle() 和更新传感器值
 *
 * 示例：
 * ```cpp
 * SeeedHADiscovery ha;
 * SeeedHASensor* tempSensor;
 *
 * void setup() {
 *     ha.begin("WiFi名称", "WiFi密码");
 *     tempSensor = ha.addSensor("temp", "温度", "temperature", "°C");
 * }
 *
 * void loop() {
 *     ha.handle();
 *     tempSensor->setValue(25.5);
 * }
 * ```
 *
 * @author limengdu
 * @version 1.0.0
 * @license MIT
 */

#ifndef SEEED_HA_DISCOVERY_H
#define SEEED_HA_DISCOVERY_H

// =============================================================================
// 依赖库 | Dependencies
// =============================================================================

#include <Arduino.h>       // Arduino 核心库
#include <WiFi.h>          // ESP32 WiFi 库
#include <WebServer.h>     // HTTP 服务器（用于提供设备信息）
#include <WebSocketsServer.h>  // WebSocket 服务器（用于实时通信）
#include <ESPmDNS.h>       // mDNS 服务（用于设备发现）
#include <ArduinoJson.h>   // JSON 处理库
#include <vector>          // 动态数组
#include <map>             // 键值对容器

// =============================================================================
// 常量定义 | Constants
// =============================================================================

// 库版本号
#define SEEED_HA_DISCOVERY_VERSION "1.0.0"

// 默认端口
#define SEEED_HA_HTTP_PORT 80   // HTTP 服务器端口（用于设备信息接口）
#define SEEED_HA_WS_PORT 81     // WebSocket 端口（用于实时通信）

// =============================================================================
// 前向声明 | Forward Declarations
// =============================================================================

class SeeedHADiscovery;
class SeeedHASensor;

// =============================================================================
// SeeedHASensor - 传感器类
// =============================================================================

/**
 * 传感器类 - 代表一个数值型传感器
 * Sensor class - represents a numeric sensor
 *
 * 用于向 Home Assistant 报告测量值，如温度、湿度等。
 * Used to report measurement values to Home Assistant.
 *
 * 支持的属性：
 * - device_class: 设备类别（temperature, humidity 等）
 * - unit_of_measurement: 单位（°C, % 等）
 * - state_class: 状态类别（measurement, total 等）
 * - precision: 小数精度
 * - icon: 图标（mdi:xxx 格式）
 */
class SeeedHASensor {
public:
    /**
     * 构造函数
     * Constructor
     *
     * @param id 传感器 ID（唯一标识符，如 "temperature"）
     * @param name 显示名称（如 "温度"）
     * @param deviceClass 设备类别（如 "temperature"）
     * @param unit 单位（如 "°C"）
     */
    SeeedHASensor(
        const String& id,
        const String& name,
        const String& deviceClass = "",
        const String& unit = ""
    );

    // =========================================================================
    // 设置方法 | Setters
    // =========================================================================

    /**
     * 设置传感器的当前值
     * Set the current sensor value
     *
     * 调用此方法后，新值会自动推送到 Home Assistant。
     *
     * @param value 传感器值
     */
    void setValue(float value);

    /**
     * 设置状态类别
     * Set state class
     *
     * @param stateClass 状态类别：
     *        - "measurement": 测量值（默认）
     *        - "total": 总计值
     *        - "total_increasing": 只增不减的总计值
     */
    void setStateClass(const String& stateClass);

    /**
     * 设置显示精度
     * Set display precision
     *
     * @param precision 小数位数（如 1 表示显示 25.5）
     */
    void setPrecision(int precision);

    /**
     * 设置图标
     * Set icon
     *
     * @param icon 图标名称（mdi 格式，如 "mdi:thermometer"）
     */
    void setIcon(const String& icon);

    // =========================================================================
    // 获取方法 | Getters
    // =========================================================================

    const String& getId() const { return _id; }
    const String& getName() const { return _name; }
    float getValue() const { return _value; }
    const String& getDeviceClass() const { return _deviceClass; }
    const String& getUnit() const { return _unit; }
    const String& getStateClass() const { return _stateClass; }
    int getPrecision() const { return _precision; }
    const String& getIcon() const { return _icon; }

    // =========================================================================
    // 内部方法 | Internal Methods
    // =========================================================================

    /**
     * 转换为 JSON 格式（用于发送到 HA）
     * Convert to JSON format (for sending to HA)
     */
    void toJson(JsonObject& obj) const;

private:
    String _id;           // 传感器 ID
    String _name;         // 显示名称
    String _deviceClass;  // 设备类别
    String _unit;         // 单位
    String _stateClass;   // 状态类别
    String _icon;         // 图标
    float _value;         // 当前值
    int _precision;       // 显示精度
    bool _hasValue;       // 是否已设置值

    // 关联的主类实例
    SeeedHADiscovery* _ha;
    friend class SeeedHADiscovery;

    // 通知值变化
    void _notifyChange();
};

// =============================================================================
// SeeedHADiscovery - 主类
// =============================================================================

/**
 * Seeed Home Assistant Discovery 主类
 * Main class for Seeed Home Assistant Discovery
 *
 * 这是库的核心类，负责：
 * - WiFi 连接管理
 * - mDNS 服务发布
 * - HTTP 服务器（设备信息）
 * - WebSocket 服务器（实时通信）
 * - 传感器管理
 */
class SeeedHADiscovery {
public:
    /**
     * 构造函数
     * Constructor
     */
    SeeedHADiscovery();

    /**
     * 析构函数
     * Destructor
     */
    ~SeeedHADiscovery();

    // =========================================================================
    // 配置方法 | Configuration Methods
    // =========================================================================

    /**
     * 设置设备信息
     * Set device information
     *
     * 这些信息会显示在 Home Assistant 的设备页面。
     *
     * @param name 设备名称（如 "客厅温湿度传感器"）
     * @param model 设备型号（如 "ESP32-C3"）
     * @param version 固件版本（如 "1.0.0"）
     */
    void setDeviceInfo(const String& name, const String& model, const String& version = SEEED_HA_DISCOVERY_VERSION);

    /**
     * 启用调试输出
     * Enable debug output
     *
     * 启用后，会通过 Serial 输出调试信息。
     *
     * @param enable 是否启用调试（默认 true）
     */
    void enableDebug(bool enable = true);

    // =========================================================================
    // 连接方法 | Connection Methods
    // =========================================================================

    /**
     * 连接 WiFi 并启动服务
     * Connect to WiFi and start services
     *
     * 这个方法会：
     * 1. 连接到指定的 WiFi 网络
     * 2. 启动 mDNS 服务（用于设备发现）
     * 3. 启动 HTTP 服务器（用于设备信息）
     * 4. 启动 WebSocket 服务器（用于实时通信）
     *
     * @param ssid WiFi 名称
     * @param password WiFi 密码
     * @return 连接是否成功
     */
    bool begin(const char* ssid, const char* password);

    // =========================================================================
    // 传感器管理 | Sensor Management
    // =========================================================================

    /**
     * 添加一个传感器
     * Add a sensor
     *
     * 创建一个新的传感器实体，会自动注册到 Home Assistant。
     *
     * @param id 传感器 ID（唯一标识符，如 "temperature"）
     * @param name 显示名称（如 "温度"）
     * @param deviceClass 设备类别（如 "temperature", "humidity"）
     *                    参考: https://www.home-assistant.io/integrations/sensor/#device-class
     * @param unit 单位（如 "°C", "%"）
     * @return 传感器对象指针
     *
     * 常用设备类别：
     * - temperature: 温度
     * - humidity: 湿度
     * - pressure: 气压
     * - illuminance: 光照
     * - battery: 电池电量
     * - voltage: 电压
     * - current: 电流
     * - power: 功率
     */
    SeeedHASensor* addSensor(
        const String& id,
        const String& name,
        const String& deviceClass = "",
        const String& unit = ""
    );

    // =========================================================================
    // 运行时方法 | Runtime Methods
    // =========================================================================

    /**
     * 处理网络事件
     * Handle network events
     *
     * 必须在 loop() 中调用！
     * Must be called in loop()!
     *
     * 这个方法处理：
     * - HTTP 请求
     * - WebSocket 消息
     * - 心跳检测
     */
    void handle();

    // =========================================================================
    // 状态查询 | Status Queries
    // =========================================================================

    /**
     * 检查是否已连接 WiFi
     * Check if WiFi is connected
     */
    bool isWiFiConnected() const;

    /**
     * 检查是否有 Home Assistant 连接
     * Check if Home Assistant is connected
     */
    bool isHAConnected() const;

    /**
     * 获取本机 IP 地址
     * Get local IP address
     */
    IPAddress getLocalIP() const;

    /**
     * 获取设备 ID
     * Get device ID
     */
    const String& getDeviceId() const { return _deviceId; }

    // =========================================================================
    // 内部方法（传感器类使用）| Internal Methods (used by sensor class)
    // =========================================================================

    void _notifySensorChange(const String& sensorId);

private:
    // -------------------------------------------------------------------------
    // 设备信息 | Device Information
    // -------------------------------------------------------------------------
    String _deviceName;    // 设备名称
    String _deviceModel;   // 设备型号
    String _deviceVersion; // 固件版本
    String _deviceId;      // 设备唯一 ID（基于 MAC 地址）

    // -------------------------------------------------------------------------
    // 网络服务 | Network Services
    // -------------------------------------------------------------------------
    WebServer* _httpServer;         // HTTP 服务器
    WebSocketsServer* _wsServer;    // WebSocket 服务器
    bool _wsClientConnected;        // 是否有 WebSocket 客户端连接

    // -------------------------------------------------------------------------
    // 传感器列表 | Sensor List
    // -------------------------------------------------------------------------
    std::vector<SeeedHASensor*> _sensors;

    // -------------------------------------------------------------------------
    // 状态变量 | State Variables
    // -------------------------------------------------------------------------
    bool _debug;                  // 调试模式
    unsigned long _lastHeartbeat; // 上次心跳时间

    // -------------------------------------------------------------------------
    // 内部方法 | Internal Methods
    // -------------------------------------------------------------------------

    // 生成设备 ID（基于 MAC 地址）
    String _generateDeviceId();

    // 设置 mDNS 服务
    void _setupMDNS();

    // 设置 HTTP 服务器
    void _setupHTTP();

    // 设置 WebSocket 服务器
    void _setupWebSocket();

    // HTTP 请求处理
    void _handleHTTPRoot();     // 主页
    void _handleHTTPInfo();     // 设备信息接口

    // WebSocket 事件处理
    void _handleWSEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);

    // 发送发现信息（设备支持的传感器列表）
    void _sendDiscovery(uint8_t clientNum = 255);

    // 发送传感器状态更新
    void _sendSensorState(const String& sensorId, uint8_t clientNum = 255);

    // 广播消息到所有 WebSocket 客户端
    void _broadcastMessage(const String& message);

    // 调试日志输出
    void _log(const String& message);
};

#endif // SEEED_HA_DISCOVERY_H
