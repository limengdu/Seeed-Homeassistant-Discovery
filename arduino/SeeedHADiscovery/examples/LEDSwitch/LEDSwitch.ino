/**
 * ============================================================================
 * Seeed HA Discovery - LED 开关示例
 * LED Switch Example
 * ============================================================================
 *
 * 这个示例展示如何：
 * 1. 创建一个开关实体控制板载 LED
 * 2. 接收来自 Home Assistant 的开关命令
 * 3. 同时包含传感器上报（温湿度）
 *
 * 硬件要求：
 * - XIAO ESP32-C3/C6/S3 或其他 ESP32 开发板
 * - 板载 LED（使用 LED_BUILTIN 宏）
 *
 * 软件依赖：
 * - ArduinoJson (作者: Benoit Blanchon)
 * - WebSockets (作者: Markus Sattler)
 *
 * 使用方法：
 * 1. 修改下方的 WiFi 配置
 * 2. 上传到 ESP32
 * 3. 打开串口监视器查看 IP 地址
 * 4. 在 Home Assistant 中添加设备
 * 5. 在 HA 界面控制 LED 开关
 *
 * @author limengdu
 * @version 1.0.0
 */

#include <SeeedHADiscovery.h>

// =============================================================================
// 配置区域 - 请根据你的环境修改
// Configuration - Please modify according to your environment
// =============================================================================

// WiFi 配置
// WiFi configuration
const char* WIFI_SSID = "你的WiFi名称";      // Your WiFi SSID
const char* WIFI_PASSWORD = "你的WiFi密码";  // Your WiFi password

// LED 引脚配置
// LED pin configuration
// 使用 LED_BUILTIN 宏确保跨平台兼容性
// Use LED_BUILTIN macro for cross-platform compatibility
#ifndef LED_BUILTIN
  #define LED_BUILTIN 2  // 如果未定义，默认使用 GPIO2
#endif

// LED 极性配置
// LED polarity configuration
// 有些开发板的 LED 是低电平点亮（如 XIAO ESP32-C3）
// Some boards have active-low LEDs (like XIAO ESP32-C3)
#define LED_ACTIVE_LOW false  // 设为 true 如果你的 LED 是低电平点亮

// =============================================================================
// 全局变量
// Global variables
// =============================================================================

// Seeed HA Discovery 主实例
SeeedHADiscovery ha;

// 开关实体 - 用于控制 LED
SeeedHASwitch* ledSwitch;

// 传感器实体 - 模拟温湿度（可选，展示混合使用）
SeeedHASensor* tempSensor;
SeeedHASensor* humiditySensor;

// =============================================================================
// 辅助函数
// Helper functions
// =============================================================================

/**
 * 设置 LED 状态
 * Set LED state
 *
 * @param on 是否点亮
 */
void setLED(bool on) {
    if (LED_ACTIVE_LOW) {
        // 低电平点亮
        digitalWrite(LED_BUILTIN, on ? LOW : HIGH);
    } else {
        // 高电平点亮
        digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
    }
}

// =============================================================================
// Arduino 主程序
// Arduino main program
// =============================================================================

void setup() {
    // -------------------------------------------------------------------------
    // 初始化串口
    // Initialize serial
    // -------------------------------------------------------------------------
    Serial.begin(115200);
    delay(1000);  // 等待串口稳定

    Serial.println();
    Serial.println("========================================");
    Serial.println("  Seeed HA Discovery - LED 开关示例");
    Serial.println("========================================");
    Serial.println();

    // -------------------------------------------------------------------------
    // 初始化 LED 引脚
    // Initialize LED pin
    // -------------------------------------------------------------------------
    pinMode(LED_BUILTIN, OUTPUT);
    setLED(false);  // 初始状态为关闭

    Serial.printf("LED 引脚: GPIO%d\n", LED_BUILTIN);
    Serial.printf("LED 极性: %s\n", LED_ACTIVE_LOW ? "低电平点亮" : "高电平点亮");

    // -------------------------------------------------------------------------
    // 配置 Seeed HA Discovery
    // Configure Seeed HA Discovery
    // -------------------------------------------------------------------------

    // 设置设备信息（会显示在 Home Assistant 中）
    ha.setDeviceInfo(
        "XIAO LED 控制器",   // 设备名称
        "XIAO ESP32-C3",     // 设备型号
        "1.0.0"              // 固件版本
    );

    // 启用调试输出
    ha.enableDebug(true);

    // -------------------------------------------------------------------------
    // 连接 WiFi
    // Connect to WiFi
    // -------------------------------------------------------------------------
    Serial.println("正在连接 WiFi...");

    if (!ha.begin(WIFI_SSID, WIFI_PASSWORD)) {
        Serial.println("❌ WiFi 连接失败！请检查配置。");
        Serial.println("程序停止。请重启设备重试。");
        while (1) {
            // LED 快闪表示错误
            setLED(true);
            delay(200);
            setLED(false);
            delay(200);
        }
    }

    Serial.println("✅ WiFi 连接成功！");
    Serial.print("IP 地址: ");
    Serial.println(ha.getLocalIP());

    // -------------------------------------------------------------------------
    // 添加开关实体 - LED 控制
    // Add switch entity - LED control
    // -------------------------------------------------------------------------

    // 创建 LED 开关
    // 参数: ID, 名称, 图标（可选）
    ledSwitch = ha.addSwitch("led", "板载LED", "mdi:led-on");

    // 注册状态变化回调
    // 当 Home Assistant 发送开关命令时，这个回调会被调用
    ledSwitch->onStateChange([](bool state) {
        Serial.printf("📍 收到 LED 命令: %s\n", state ? "开启" : "关闭");

        // 执行实际的硬件操作
        setLED(state);

        Serial.printf("💡 LED 已%s\n", state ? "点亮" : "熄灭");
    });

    Serial.println("✅ LED 开关已注册");

    // -------------------------------------------------------------------------
    // 添加传感器实体（可选）- 模拟温湿度
    // Add sensor entities (optional) - Simulated temperature & humidity
    // -------------------------------------------------------------------------

    // 创建温度传感器
    tempSensor = ha.addSensor("temperature", "温度", "temperature", "°C");
    tempSensor->setPrecision(1);  // 1 位小数

    // 创建湿度传感器
    humiditySensor = ha.addSensor("humidity", "湿度", "humidity", "%");
    humiditySensor->setPrecision(0);  // 整数

    Serial.println("✅ 传感器已注册");

    // -------------------------------------------------------------------------
    // 完成初始化
    // Initialization complete
    // -------------------------------------------------------------------------

    Serial.println();
    Serial.println("========================================");
    Serial.println("  初始化完成！");
    Serial.println("========================================");
    Serial.println();
    Serial.println("📱 请在 Home Assistant 中添加此设备");
    Serial.println("   设置 → 设备与服务 → 添加集成");
    Serial.println("   搜索 'Seeed HA Discovery'");
    Serial.printf("   输入 IP: %s\n", ha.getLocalIP().toString().c_str());
    Serial.println();
    Serial.println("🔗 或访问设备状态页面:");
    Serial.printf("   http://%s\n", ha.getLocalIP().toString().c_str());
    Serial.println();
}

void loop() {
    // -------------------------------------------------------------------------
    // 必须调用！处理网络事件
    // Must be called! Handle network events
    // -------------------------------------------------------------------------
    ha.handle();

    // -------------------------------------------------------------------------
    // 模拟传感器数据更新（每 10 秒）
    // Simulate sensor data update (every 10 seconds)
    // -------------------------------------------------------------------------
    static unsigned long lastSensorUpdate = 0;
    if (millis() - lastSensorUpdate > 10000) {
        lastSensorUpdate = millis();

        // 生成模拟数据
        float temp = 22.0 + random(-50, 51) / 10.0;  // 17.0 ~ 27.0
        float humidity = 50.0 + random(-20, 21);      // 30 ~ 70

        // 更新传感器值（会自动推送到 HA）
        tempSensor->setValue(temp);
        humiditySensor->setValue(humidity);

        Serial.printf("📊 传感器更新: 温度=%.1f°C, 湿度=%.0f%%\n", temp, humidity);
    }

    // -------------------------------------------------------------------------
    // 连接状态监控
    // Connection status monitoring
    // -------------------------------------------------------------------------
    static unsigned long lastStatusCheck = 0;
    static bool wasConnected = false;

    if (millis() - lastStatusCheck > 5000) {
        lastStatusCheck = millis();

        bool isConnected = ha.isHAConnected();

        if (isConnected != wasConnected) {
            if (isConnected) {
                Serial.println("🟢 Home Assistant 已连接");
            } else {
                Serial.println("🔴 Home Assistant 已断开");
            }
            wasConnected = isConnected;
        }
    }
}

