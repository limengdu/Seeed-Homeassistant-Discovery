/**
 * ============================================================================
 * Seeed HA Discovery - 温湿度传感器示例
 * Temperature & Humidity Sensor Example
 * ============================================================================
 *
 * 这个示例展示如何：
 * 1. 将 ESP32 连接到 WiFi
 * 2. 创建温度和湿度传感器
 * 3. 向 Home Assistant 实时上报传感器数据
 *
 * 硬件要求：
 * - XIAO ESP32-C3/C6/S3 或其他 ESP32 开发板
 * - DHT22 温湿度传感器（可选，本示例也可使用模拟数据）
 *
 * DHT22 接线说明：
 * - VCC  → 3.3V
 * - GND  → GND
 * - DATA → D2 (可在下方修改)
 *
 * 软件依赖：
 * - ArduinoJson (作者: Benoit Blanchon)
 * - WebSockets (作者: Markus Sattler)
 * - DHT sensor library (作者: Adafruit) - 如果使用 DHT22
 *
 * 使用方法：
 * 1. 修改下方的 WiFi 配置
 * 2. 如果使用 DHT22，取消注释 USE_DHT_SENSOR
 * 3. 上传到 ESP32
 * 4. 打开串口监视器查看 IP 地址
 * 5. 在 Home Assistant 中添加设备
 *
 * @author limengdu
 * @version 1.0.0
 */

#include <SeeedHADiscovery.h>

// 如果使用 DHT22 传感器，取消下面两行注释
// #include <DHT.h>
// #define USE_DHT_SENSOR

// =============================================================================
// 配置区域 - 请根据你的环境修改
// Configuration - Please modify according to your environment
// =============================================================================

// WiFi 配置
const char* WIFI_SSID = "你的WiFi名称";      // Your WiFi SSID
const char* WIFI_PASSWORD = "你的WiFi密码";  // Your WiFi password

// DHT 传感器配置（如果使用）
#ifdef USE_DHT_SENSOR
    #define DHT_PIN D2        // DHT 数据引脚
    #define DHT_TYPE DHT22    // DHT 类型: DHT11 或 DHT22
    DHT dht(DHT_PIN, DHT_TYPE);
#endif

// 数据上报间隔（毫秒）
const unsigned long UPDATE_INTERVAL = 5000;  // 5 秒

// =============================================================================
// 全局变量
// =============================================================================

SeeedHADiscovery ha;
SeeedHASensor* tempSensor;
SeeedHASensor* humiditySensor;

// =============================================================================
// 辅助函数
// =============================================================================

/**
 * 读取温度值
 */
float readTemperature() {
    #ifdef USE_DHT_SENSOR
        float temp = dht.readTemperature();
        if (isnan(temp)) {
            Serial.println("DHT22 温度读取失败！");
            return 0;
        }
        return temp;
    #else
        // 模拟数据：20-30°C 之间波动
        static float baseTemp = 25.0;
        baseTemp += (random(-10, 11)) / 100.0;
        if (baseTemp < 20) baseTemp = 20;
        if (baseTemp > 30) baseTemp = 30;
        return baseTemp;
    #endif
}

/**
 * 读取湿度值
 */
float readHumidity() {
    #ifdef USE_DHT_SENSOR
        float humidity = dht.readHumidity();
        if (isnan(humidity)) {
            Serial.println("DHT22 湿度读取失败！");
            return 0;
        }
        return humidity;
    #else
        // 模拟数据：40-70% 之间波动
        static float baseHumidity = 55.0;
        baseHumidity += (random(-10, 11)) / 50.0;
        if (baseHumidity < 40) baseHumidity = 40;
        if (baseHumidity > 70) baseHumidity = 70;
        return baseHumidity;
    #endif
}

// =============================================================================
// Arduino 主程序
// =============================================================================

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("========================================");
    Serial.println("  Seeed HA Discovery - 温湿度传感器示例");
    Serial.println("========================================");
    Serial.println();

    // 初始化 DHT 传感器（如果使用）
    #ifdef USE_DHT_SENSOR
        Serial.println("正在初始化 DHT22 传感器...");
        dht.begin();
        Serial.printf("DHT22 引脚: GPIO%d\n", DHT_PIN);
    #else
        Serial.println("⚠️ 使用模拟传感器数据");
        Serial.println("   如需使用 DHT22，请取消 USE_DHT_SENSOR 注释");
    #endif
    Serial.println();

    // 配置设备信息
    ha.setDeviceInfo(
        "温湿度传感器",      // 设备名称
        "XIAO ESP32",        // 设备型号
        "1.0.0"              // 固件版本
    );

    ha.enableDebug(true);

    // 连接 WiFi
    Serial.println("正在连接 WiFi...");

    if (!ha.begin(WIFI_SSID, WIFI_PASSWORD)) {
        Serial.println("❌ WiFi 连接失败！");
        while (1) delay(1000);
    }

    Serial.println("✅ WiFi 连接成功！");
    Serial.printf("IP 地址: %s\n", ha.getLocalIP().toString().c_str());

    // =========================================================================
    // 创建传感器
    // =========================================================================

    // 温度传感器
    tempSensor = ha.addSensor("temperature", "温度", "temperature", "°C");
    tempSensor->setPrecision(1);  // 1 位小数

    // 湿度传感器
    humiditySensor = ha.addSensor("humidity", "湿度", "humidity", "%");
    humiditySensor->setPrecision(0);  // 整数

    // =========================================================================
    // 完成初始化
    // =========================================================================

    Serial.println();
    Serial.println("========================================");
    Serial.println("  初始化完成！");
    Serial.println("========================================");
    Serial.println();
    Serial.println("在 Home Assistant 中添加设备:");
    Serial.println("  设置 → 设备与服务 → 添加集成");
    Serial.println("  搜索 'Seeed HA Discovery'");
    Serial.printf("  输入 IP: %s\n", ha.getLocalIP().toString().c_str());
    Serial.println();
    Serial.printf("设备状态页面: http://%s\n", ha.getLocalIP().toString().c_str());
    Serial.println();
}

void loop() {
    // 必须调用！处理网络事件
    ha.handle();

    // 定期读取并上报传感器数据
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > UPDATE_INTERVAL) {
        lastUpdate = millis();

        float temp = readTemperature();
        float humidity = readHumidity();

        // 更新传感器值（自动推送到 HA）
        tempSensor->setValue(temp);
        humiditySensor->setValue(humidity);

        Serial.printf("📊 温度: %.1f°C, 湿度: %.0f%%\n", temp, humidity);
    }

    // 连接状态监控
    static unsigned long lastCheck = 0;
    static bool wasConnected = false;

    if (millis() - lastCheck > 5000) {
        lastCheck = millis();

        bool connected = ha.isHAConnected();
        if (connected != wasConnected) {
            Serial.println(connected ? "🟢 HA 已连接" : "🔴 HA 已断开");
            wasConnected = connected;
        }
    }
}
