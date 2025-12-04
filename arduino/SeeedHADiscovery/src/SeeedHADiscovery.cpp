/**
 * ============================================================================
 * Seeed Home Assistant Discovery - 实现文件
 * Seeed Home Assistant Discovery - Implementation file
 * ============================================================================
 *
 * 这个文件包含 SeeedHADiscovery 和 SeeedHASensor 类的实现。
 *
 * @author limengdu
 */

#include "SeeedHADiscovery.h"

// =============================================================================
// SeeedHASensor 实现 | SeeedHASensor Implementation
// =============================================================================

SeeedHASensor::SeeedHASensor(
    const String& id,
    const String& name,
    const String& deviceClass,
    const String& unit
) :
    _id(id),
    _name(name),
    _deviceClass(deviceClass),
    _unit(unit),
    _stateClass("measurement"),  // 默认状态类别
    _icon(""),
    _value(0),
    _precision(1),               // 默认 1 位小数
    _hasValue(false),
    _ha(nullptr)
{
    // 构造函数初始化完成
}

void SeeedHASensor::setValue(float value) {
    // 设置传感器值
    _value = value;
    _hasValue = true;

    // 通知主类，值已更新
    _notifyChange();
}

void SeeedHASensor::setStateClass(const String& stateClass) {
    _stateClass = stateClass;
}

void SeeedHASensor::setPrecision(int precision) {
    _precision = precision;
}

void SeeedHASensor::setIcon(const String& icon) {
    _icon = icon;
}

void SeeedHASensor::toJson(JsonObject& obj) const {
    // 将传感器信息转换为 JSON 格式
    // 这个 JSON 会发送给 Home Assistant

    obj["id"] = _id;                   // 传感器 ID
    obj["name"] = _name;               // 显示名称
    obj["type"] = "sensor";            // 实体类型（固定为 sensor）

    // 设备类别（如 temperature, humidity）
    if (_deviceClass.length() > 0) {
        obj["device_class"] = _deviceClass;
    }

    // 单位（如 °C, %）
    if (_unit.length() > 0) {
        obj["unit_of_measurement"] = _unit;
    }

    // 状态类别（如 measurement, total）
    obj["state_class"] = _stateClass;

    // 显示精度
    obj["precision"] = _precision;

    // 图标
    if (_icon.length() > 0) {
        obj["icon"] = _icon;
    }

    // 当前值（如果已设置）
    if (_hasValue) {
        obj["state"] = _value;
    }
}

void SeeedHASensor::_notifyChange() {
    // 通知主类，传感器值已更新
    if (_ha != nullptr) {
        _ha->_notifySensorChange(_id);
    }
}

// =============================================================================
// SeeedHASwitch 实现 | SeeedHASwitch Implementation
// =============================================================================

SeeedHASwitch::SeeedHASwitch(
    const String& id,
    const String& name,
    const String& icon
) :
    _id(id),
    _name(name),
    _icon(icon),
    _state(false),
    _callback(nullptr),
    _ha(nullptr)
{
    // 构造函数初始化完成
}

void SeeedHASwitch::setState(bool state) {
    // 如果状态没有变化，不做任何事
    if (_state == state) {
        return;
    }

    // 更新状态
    _state = state;

    // 通知主类，状态已更新（会发送到 HA）
    _notifyChange();
}

void SeeedHASwitch::toggle() {
    // 切换状态
    setState(!_state);
}

void SeeedHASwitch::onStateChange(SwitchCallback callback) {
    // 注册回调函数
    _callback = callback;
}

void SeeedHASwitch::setIcon(const String& icon) {
    _icon = icon;
}

void SeeedHASwitch::toJson(JsonObject& obj) const {
    // 将开关信息转换为 JSON 格式
    // 这个 JSON 会发送给 Home Assistant

    obj["id"] = _id;           // 开关 ID
    obj["name"] = _name;       // 显示名称
    obj["type"] = "switch";    // 实体类型（固定为 switch）
    obj["state"] = _state;     // 当前状态

    // 图标
    if (_icon.length() > 0) {
        obj["icon"] = _icon;
    }
}

void SeeedHASwitch::_handleCommand(bool state) {
    // 处理来自 HA 的命令

    // 先更新状态（但不通知，避免循环）
    _state = state;

    // 如果有回调，调用回调让用户处理硬件操作
    if (_callback != nullptr) {
        _callback(state);
    }

    // 通知主类发送状态确认给 HA
    _notifyChange();
}

void SeeedHASwitch::_notifyChange() {
    // 通知主类，开关状态已更新
    if (_ha != nullptr) {
        _ha->_notifySwitchChange(_id);
    }
}

// =============================================================================
// SeeedHADiscovery 实现 | SeeedHADiscovery Implementation
// =============================================================================

SeeedHADiscovery::SeeedHADiscovery() :
    _deviceName("Seeed HA 设备"),
    _deviceModel("ESP32"),
    _deviceVersion(SEEED_HA_DISCOVERY_VERSION),
    _httpServer(nullptr),
    _wsServer(nullptr),
    _wsClientConnected(false),
    _debug(false),
    _lastHeartbeat(0)
{
    // 生成设备 ID
    _deviceId = _generateDeviceId();
}

SeeedHADiscovery::~SeeedHADiscovery() {
    // 清理 HTTP 服务器
    if (_httpServer != nullptr) {
        _httpServer->stop();
        delete _httpServer;
    }

    // 清理 WebSocket 服务器
    if (_wsServer != nullptr) {
        _wsServer->close();
        delete _wsServer;
    }

    // 清理传感器
    for (auto sensor : _sensors) {
        delete sensor;
    }
    _sensors.clear();

    // 清理开关
    for (auto sw : _switches) {
        delete sw;
    }
    _switches.clear();
}

void SeeedHADiscovery::setDeviceInfo(const String& name, const String& model, const String& version) {
    _deviceName = name;
    _deviceModel = model;
    _deviceVersion = version;
}

void SeeedHADiscovery::enableDebug(bool enable) {
    _debug = enable;
}

bool SeeedHADiscovery::begin(const char* ssid, const char* password) {
    _log("====================================");
    _log("Seeed HA Discovery 启动中...");
    _log("====================================");

    // -------------------------------------------------------------------------
    // 步骤 1: 连接 WiFi
    // -------------------------------------------------------------------------
    _log("正在连接 WiFi: " + String(ssid));

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    // 等待连接（最多 30 秒）
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 60) {
        delay(500);
        if (_debug) {
            Serial.print(".");
        }
        attempts++;
    }

    if (_debug) {
        Serial.println();
    }

    // 检查连接结果
    if (WiFi.status() != WL_CONNECTED) {
        _log("WiFi 连接失败！");
        return false;
    }

    _log("WiFi 连接成功！");
    _log("IP 地址: " + WiFi.localIP().toString());
    _log("设备 ID: " + _deviceId);

    // -------------------------------------------------------------------------
    // 步骤 2: 启动 mDNS 服务（用于设备自动发现）
    // -------------------------------------------------------------------------
    _setupMDNS();

    // -------------------------------------------------------------------------
    // 步骤 3: 启动 HTTP 服务器（提供设备信息接口）
    // -------------------------------------------------------------------------
    _setupHTTP();

    // -------------------------------------------------------------------------
    // 步骤 4: 启动 WebSocket 服务器（用于实时通信）
    // -------------------------------------------------------------------------
    _setupWebSocket();

    _log("====================================");
    _log("所有服务已启动！");
    _log("在浏览器中打开: http://" + WiFi.localIP().toString());
    _log("====================================");

    return true;
}

void SeeedHADiscovery::_setupMDNS() {
    // 生成主机名（基于设备 ID）
    String hostname = "seeed-ha-" + _deviceId;
    hostname.toLowerCase();

    _log("启动 mDNS 服务: " + hostname + ".local");

    if (MDNS.begin(hostname.c_str())) {
        // 注册 _seeed_ha._tcp 服务，这样 Home Assistant 就能发现设备
        MDNS.addService("seeed_ha", "tcp", SEEED_HA_WS_PORT);

        // 添加 TXT 记录，包含设备信息
        MDNS.addServiceTxt("seeed_ha", "tcp", "id", _deviceId);
        MDNS.addServiceTxt("seeed_ha", "tcp", "name", _deviceName);
        MDNS.addServiceTxt("seeed_ha", "tcp", "model", _deviceModel);
        MDNS.addServiceTxt("seeed_ha", "tcp", "version", _deviceVersion);

        _log("mDNS 服务已启动");
    } else {
        _log("mDNS 启动失败！");
    }
}

void SeeedHADiscovery::_setupHTTP() {
    _httpServer = new WebServer(SEEED_HA_HTTP_PORT);

    // 注册路由处理器

    // 主页 - 显示设备状态页面
    _httpServer->on("/", HTTP_GET, [this]() {
        _handleHTTPRoot();
    });

    // 设备信息接口 - 返回 JSON 格式的设备信息
    _httpServer->on("/info", HTTP_GET, [this]() {
        _handleHTTPInfo();
    });

    // 启动服务器
    _httpServer->begin();
    _log("HTTP 服务器已启动，端口: " + String(SEEED_HA_HTTP_PORT));
}

void SeeedHADiscovery::_setupWebSocket() {
    _wsServer = new WebSocketsServer(SEEED_HA_WS_PORT);

    // 注册 WebSocket 事件处理器
    _wsServer->onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
        _handleWSEvent(num, type, payload, length);
    });

    // 启动服务器
    _wsServer->begin();
    _log("WebSocket 服务器已启动，端口: " + String(SEEED_HA_WS_PORT));
}

void SeeedHADiscovery::_handleHTTPRoot() {
    // 生成一个简单的状态页面
    String html = R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Seeed HA Discovery</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
            min-height: 100vh;
            padding: 20px;
            color: #eee;
        }
        .container { max-width: 600px; margin: 0 auto; }
        h1 {
            color: #00d9ff;
            margin-bottom: 20px;
            font-size: 2em;
        }
        .card {
            background: rgba(255,255,255,0.1);
            border-radius: 12px;
            padding: 20px;
            margin-bottom: 20px;
            backdrop-filter: blur(10px);
        }
        .card h2 {
            color: #00d9ff;
            margin-bottom: 15px;
            font-size: 1.2em;
        }
        .info-row {
            display: flex;
            justify-content: space-between;
            padding: 10px 0;
            border-bottom: 1px solid rgba(255,255,255,0.1);
        }
        .info-row:last-child { border-bottom: none; }
        .label { color: #888; }
        .value { color: #fff; font-weight: 500; }
        .sensor {
            background: rgba(0,217,255,0.1);
            border-radius: 8px;
            padding: 15px;
            margin-bottom: 10px;
        }
        .sensor-name {
            font-weight: 600;
            color: #00d9ff;
        }
        .sensor-value {
            font-size: 1.5em;
            margin-top: 5px;
        }
        .sensor-unit {
            color: #888;
            font-size: 0.8em;
        }
        .status {
            display: inline-block;
            padding: 4px 12px;
            border-radius: 20px;
            font-size: 0.9em;
        }
        .status.online, .status.on {
            background: rgba(0,255,136,0.2);
            color: #00ff88;
        }
        .status.off {
            background: rgba(255,100,100,0.2);
            color: #ff6464;
        }
        .footer {
            text-align: center;
            color: #666;
            font-size: 0.9em;
            margin-top: 20px;
        }
        .logo {
            color: #00d9ff;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌱 Seeed HA Discovery</h1>

        <div class="card">
            <h2>设备信息</h2>
            <div class="info-row">
                <span class="label">设备名称</span>
                <span class="value">)" + _deviceName + R"(</span>
            </div>
            <div class="info-row">
                <span class="label">设备型号</span>
                <span class="value">)" + _deviceModel + R"(</span>
            </div>
            <div class="info-row">
                <span class="label">固件版本</span>
                <span class="value">)" + _deviceVersion + R"(</span>
            </div>
            <div class="info-row">
                <span class="label">设备 ID</span>
                <span class="value">)" + _deviceId + R"(</span>
            </div>
            <div class="info-row">
                <span class="label">IP 地址</span>
                <span class="value">)" + WiFi.localIP().toString() + R"(</span>
            </div>
            <div class="info-row">
                <span class="label">Home Assistant</span>
                <span class="status )" + String(_wsClientConnected ? "online" : "") + R"(">
                    )" + String(_wsClientConnected ? "已连接" : "等待连接") + R"(
                </span>
            </div>
        </div>

        <div class="card">
            <h2>传感器</h2>)";

    // 添加传感器列表
    if (_sensors.empty()) {
        html += R"(
            <p style="color: #888;">暂无传感器</p>)";
    } else {
        for (auto sensor : _sensors) {
            html += R"(
            <div class="sensor">
                <div class="sensor-name">)" + sensor->getName() + R"(</div>
                <div class="sensor-value">
                    )" + String(sensor->getValue(), sensor->getPrecision()) + R"(
                    <span class="sensor-unit">)" + sensor->getUnit() + R"(</span>
                </div>
            </div>)";
        }
    }

    html += R"(
        </div>

        <div class="card">
            <h2>开关</h2>)";

    // 添加开关列表
    if (_switches.empty()) {
        html += R"(
            <p style="color: #888;">暂无开关</p>)";
    } else {
        for (auto sw : _switches) {
            String stateClass = sw->getState() ? "on" : "off";
            String stateText = sw->getState() ? "开启" : "关闭";
            html += R"(
            <div class="sensor">
                <div class="sensor-name">)" + sw->getName() + R"(</div>
                <div class="sensor-value">
                    <span class="status )" + stateClass + R"(">)" + stateText + R"(</span>
                </div>
            </div>)";
        }
    }

    html += R"(
        </div>

        <div class="footer">
            <span class="logo">Seeed Studio</span> | Seeed HA Discovery v)" + String(SEEED_HA_DISCOVERY_VERSION) + R"(
        </div>
    </div>
</body>
</html>)";

    _httpServer->send(200, "text/html; charset=utf-8", html);
}

void SeeedHADiscovery::_handleHTTPInfo() {
    // 返回 JSON 格式的设备信息
    // Home Assistant 会调用这个接口获取设备信息

    JsonDocument doc;

    doc["device_id"] = _deviceId;
    doc["name"] = _deviceName;
    doc["model"] = _deviceModel;
    doc["version"] = _deviceVersion;
    doc["ip"] = WiFi.localIP().toString();
    doc["mac"] = WiFi.macAddress();
    doc["rssi"] = WiFi.RSSI();

    String response;
    serializeJson(doc, response);

    _httpServer->send(200, "application/json", response);
}

void SeeedHADiscovery::_handleWSEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            // 客户端断开连接
            _log("WebSocket 客户端 #" + String(num) + " 断开连接");
            _wsClientConnected = false;
            break;

        case WStype_CONNECTED: {
            // 新客户端连接
            IPAddress ip = _wsServer->remoteIP(num);
            _log("WebSocket 客户端 #" + String(num) + " 已连接，来自 " + ip.toString());
            _wsClientConnected = true;

            // 向新客户端发送发现信息
            _sendDiscovery(num);
            break;
        }

        case WStype_TEXT: {
            // 收到文本消息
            String message = String((char*)payload);
            _log("收到消息: " + message);

            // 解析 JSON
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, message);

            if (error) {
                _log("JSON 解析错误: " + String(error.c_str()));
                return;
            }

            // 获取消息类型
            String msgType = doc["type"].as<String>();

            if (msgType == "ping") {
                // 心跳请求，回复 pong
                JsonDocument response;
                response["type"] = "pong";
                response["timestamp"] = doc["timestamp"];

                String responseStr;
                serializeJson(response, responseStr);
                _wsServer->sendTXT(num, responseStr);
            }
            else if (msgType == "discovery") {
                // 发现请求，发送实体列表
                _sendDiscovery(num);
            }
            else if (msgType == "command") {
                // 来自 HA 的控制命令
                // 格式: {type: "command", entity_id: "led", command: "turn_on"} 或
                //       {type: "command", entity_id: "led", state: true}
                _handleCommand(doc);
            }
            break;
        }

        default:
            break;
    }
}

void SeeedHADiscovery::_sendDiscovery(uint8_t clientNum) {
    // 构建发现消息
    JsonDocument doc;
    doc["type"] = "discovery";

    JsonArray entities = doc["entities"].to<JsonArray>();

    // 添加所有传感器
    for (auto sensor : _sensors) {
        JsonObject obj = entities.add<JsonObject>();
        sensor->toJson(obj);
    }

    // 添加所有开关
    for (auto sw : _switches) {
        JsonObject obj = entities.add<JsonObject>();
        sw->toJson(obj);
    }

    // 序列化并发送
    String message;
    serializeJson(doc, message);

    if (clientNum == 255) {
        // 广播给所有客户端
        _broadcastMessage(message);
    } else {
        // 发送给指定客户端
        _wsServer->sendTXT(clientNum, message);
    }

    _log("已发送发现信息: " + String(_sensors.size()) + " 个传感器, " +
         String(_switches.size()) + " 个开关");
}

void SeeedHADiscovery::_sendSensorState(const String& sensorId, uint8_t clientNum) {
    // 查找传感器
    SeeedHASensor* sensor = nullptr;
    for (auto s : _sensors) {
        if (s->getId() == sensorId) {
            sensor = s;
            break;
        }
    }

    if (sensor == nullptr) {
        return;
    }

    // 构建状态更新消息
    JsonDocument doc;
    doc["type"] = "state";
    doc["entity_id"] = sensorId;
    doc["state"] = sensor->getValue();

    // 可以添加额外属性
    JsonObject attrs = doc["attributes"].to<JsonObject>();
    attrs["unit_of_measurement"] = sensor->getUnit();
    attrs["device_class"] = sensor->getDeviceClass();

    // 序列化并发送
    String message;
    serializeJson(doc, message);

    if (clientNum == 255) {
        _broadcastMessage(message);
    } else {
        _wsServer->sendTXT(clientNum, message);
    }

    _log("发送状态更新: " + sensorId + " = " + String(sensor->getValue()));
}

void SeeedHADiscovery::_broadcastMessage(const String& message) {
    // WebSockets 库的 broadcastTXT 需要非 const 引用，所以创建副本
    String msg = message;
    _wsServer->broadcastTXT(msg);
}

SeeedHASensor* SeeedHADiscovery::addSensor(
    const String& id,
    const String& name,
    const String& deviceClass,
    const String& unit
) {
    // 创建新传感器
    SeeedHASensor* sensor = new SeeedHASensor(id, name, deviceClass, unit);
    sensor->_ha = this;

    // 添加到列表
    _sensors.push_back(sensor);

    _log("添加传感器: " + name + " (ID: " + id + ")");

    return sensor;
}

SeeedHASwitch* SeeedHADiscovery::addSwitch(
    const String& id,
    const String& name,
    const String& icon
) {
    // 创建新开关
    SeeedHASwitch* sw = new SeeedHASwitch(id, name, icon);
    sw->_ha = this;

    // 添加到列表
    _switches.push_back(sw);

    _log("添加开关: " + name + " (ID: " + id + ")");

    return sw;
}

void SeeedHADiscovery::_notifySensorChange(const String& sensorId) {
    // 当传感器值变化时，发送状态更新
    if (_wsClientConnected) {
        _sendSensorState(sensorId);
    }
}

void SeeedHADiscovery::_notifySwitchChange(const String& switchId) {
    // 当开关状态变化时，发送状态更新
    if (_wsClientConnected) {
        _sendSwitchState(switchId);
    }
}

void SeeedHADiscovery::_sendSwitchState(const String& switchId, uint8_t clientNum) {
    // 查找开关
    SeeedHASwitch* sw = nullptr;
    for (auto s : _switches) {
        if (s->getId() == switchId) {
            sw = s;
            break;
        }
    }

    if (sw == nullptr) {
        return;
    }

    // 构建状态更新消息
    JsonDocument doc;
    doc["type"] = "state";
    doc["entity_id"] = switchId;
    doc["state"] = sw->getState();

    // 序列化并发送
    String message;
    serializeJson(doc, message);

    if (clientNum == 255) {
        _broadcastMessage(message);
    } else {
        _wsServer->sendTXT(clientNum, message);
    }

    _log("发送开关状态: " + switchId + " = " + String(sw->getState() ? "ON" : "OFF"));
}

void SeeedHADiscovery::_handleCommand(JsonDocument& doc) {
    // 处理来自 Home Assistant 的控制命令
    // 格式 1: {type: "command", entity_id: "led", command: "turn_on"}
    // 格式 2: {type: "command", entity_id: "led", state: true}

    String entityId = doc["entity_id"].as<String>();

    if (entityId.length() == 0) {
        _log("命令错误: 缺少 entity_id");
        return;
    }

    // 确定目标状态
    bool targetState = false;

    if (doc["command"].is<String>()) {
        // 格式 1: 使用命令字符串
        String command = doc["command"].as<String>();
        if (command == "turn_on") {
            targetState = true;
        } else if (command == "turn_off") {
            targetState = false;
        } else if (command == "toggle") {
            // 需要先找到开关获取当前状态
            for (auto sw : _switches) {
                if (sw->getId() == entityId) {
                    targetState = !sw->getState();
                    break;
                }
            }
        } else {
            _log("未知命令: " + command);
            return;
        }
    } else if (doc["state"].is<bool>()) {
        // 格式 2: 直接使用状态值
        targetState = doc["state"].as<bool>();
    } else {
        _log("命令错误: 缺少 command 或 state");
        return;
    }

    // 查找并执行命令
    for (auto sw : _switches) {
        if (sw->getId() == entityId) {
            _log("执行命令: " + entityId + " -> " + String(targetState ? "ON" : "OFF"));
            sw->_handleCommand(targetState);
            return;
        }
    }

    _log("未找到开关: " + entityId);
}

void SeeedHADiscovery::handle() {
    // 处理 HTTP 请求
    if (_httpServer != nullptr) {
        _httpServer->handleClient();
    }

    // 处理 WebSocket
    if (_wsServer != nullptr) {
        _wsServer->loop();
    }

    // 定期心跳（每 30 秒）
    unsigned long now = millis();
    if (now - _lastHeartbeat > 30000) {
        _lastHeartbeat = now;

        if (_wsClientConnected) {
            // 发送心跳
            JsonDocument doc;
            doc["type"] = "ping";
            doc["timestamp"] = now;

            String message;
            serializeJson(doc, message);
            _broadcastMessage(message);
        }
    }
}

bool SeeedHADiscovery::isWiFiConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

bool SeeedHADiscovery::isHAConnected() const {
    return _wsClientConnected;
}

IPAddress SeeedHADiscovery::getLocalIP() const {
    return WiFi.localIP();
}

String SeeedHADiscovery::_generateDeviceId() {
    // 使用 MAC 地址生成唯一设备 ID
    uint8_t mac[6];
    WiFi.macAddress(mac);

    char id[13];
    snprintf(id, sizeof(id), "%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return String(id);
}

void SeeedHADiscovery::_log(const String& message) {
    if (_debug) {
        Serial.println("[SeeedHA] " + message);
    }
}
