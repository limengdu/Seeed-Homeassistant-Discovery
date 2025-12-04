/**
 * ============================================================================
 * Seeed HA Discovery - 温湿度传感器示例
 * Seeed HA Discovery - Temperature and Humidity Sensor Example
 * ============================================================================
 *
 * 这个示例演示如何：
 * 1. 将 ESP32 连接到 WiFi
 * 2. 创建温度和湿度传感器
 * 3. 向 Home Assistant 上报传感器数据
 *
 * 硬件需求：
 * - ESP32-C3 / ESP32-C6 / ESP32-S3 / ESP32 开发板
 * - DHT22 温湿度传感器（可选，本示例也可使用模拟数据）
 *
 * 接线说明（DHT22）：
 * - VCC -> 3.3V
 * - GND -> GND
 * - DATA -> GPIO 4（可在下方修改）
 *
 * 使用步骤：
 * 1. 修改下方的 WiFi 名称和密码
 * 2. 如果使用 DHT22，取消注释 DHT 相关代码
 * 3. 上传程序到 ESP32
 * 4. 打开串口监视器查看输出
 * 5. 在 Home Assistant 中添加设备
 *
 * 依赖库（通过 Arduino 库管理器安装）：
 * - ArduinoJson (作者: Benoit Blanchon)
 * - WebSockets (作者: Markus Sattler)
 * - DHT sensor library (作者: Adafruit) - 如果使用 DHT22
 *
 * @author limengdu
 */

// =============================================================================
// 引入库文件
// =============================================================================

#include <SeeedHADiscovery.h>

// 如果使用 DHT22 传感器，取消下面两行注释
// #include <DHT.h>
// #define USE_DHT_SENSOR

// =============================================================================
// 配置区域 - 请根据实际情况修改
// =============================================================================

// ----- WiFi 配置 -----
// WiFi 名称（SSID）
const char* WIFI_SSID = "你的WiFi名称";

// WiFi 密码
const char* WIFI_PASSWORD = "你的WiFi密码";

// ----- 设备配置 -----
// 设备名称 - 会显示在 Home Assistant 中
const char* DEVICE_NAME = "客厅温湿度传感器";

// 设备型号 - 根据你使用的 ESP32 型号修改
// 可选: "ESP32", "ESP32-C3", "ESP32-C6", "ESP32-S3"
const char* DEVICE_MODEL = "ESP32-C3";

// 固件版本
const char* FIRMWARE_VERSION = "1.0.0";

// ----- DHT 传感器配置（如果使用）-----
#ifdef USE_DHT_SENSOR
    #define DHT_PIN 4         // DHT 数据引脚
    #define DHT_TYPE DHT22    // DHT 类型: DHT11 或 DHT22
    DHT dht(DHT_PIN, DHT_TYPE);
#endif

// ----- 数据上报间隔 -----
// 每隔多少毫秒上报一次数据
const unsigned long UPDATE_INTERVAL = 5000;  // 5 秒

// =============================================================================
// 全局变量
// =============================================================================

// Seeed HA Discovery 主对象
SeeedHADiscovery ha;

// 传感器对象指针
// 温度传感器
SeeedHASensor* temperatureSensor;

// 湿度传感器
SeeedHASensor* humiditySensor;

// 上次更新时间
unsigned long lastUpdateTime = 0;

// =============================================================================
// 辅助函数
// =============================================================================

/**
 * 读取温度值
 * 如果使用 DHT22，返回实际温度；否则返回模拟数据
 */
float readTemperature() {
    #ifdef USE_DHT_SENSOR
        // 使用 DHT22 读取温度
        float temp = dht.readTemperature();

        // 检查读取是否成功
        if (isnan(temp)) {
            Serial.println("DHT22 温度读取失败！");
            return 0;
        }
        return temp;
    #else
        // 模拟数据：20-30°C 之间随机波动
        static float baseTemp = 25.0;
        float variation = (random(-10, 11)) / 10.0;  // -1.0 到 +1.0
        baseTemp += variation * 0.1;  // 缓慢变化

        // 限制在合理范围内
        if (baseTemp < 20) baseTemp = 20;
        if (baseTemp > 30) baseTemp = 30;

        return baseTemp;
    #endif
}

/**
 * 读取湿度值
 * 如果使用 DHT22，返回实际湿度；否则返回模拟数据
 */
float readHumidity() {
    #ifdef USE_DHT_SENSOR
        // 使用 DHT22 读取湿度
        float humidity = dht.readHumidity();

        // 检查读取是否成功
        if (isnan(humidity)) {
            Serial.println("DHT22 湿度读取失败！");
            return 0;
        }
        return humidity;
    #else
        // 模拟数据：40-70% 之间随机波动
        static float baseHumidity = 55.0;
        float variation = (random(-10, 11)) / 10.0;
        baseHumidity += variation * 0.2;

        // 限制在合理范围内
        if (baseHumidity < 40) baseHumidity = 40;
        if (baseHumidity > 70) baseHumidity = 70;

        return baseHumidity;
    #endif
}

// =============================================================================
// 初始化函数 - 程序启动时执行一次
// =============================================================================

void setup() {
    // -------------------------------------------------------------------------
    // 步骤 1: 初始化串口（用于调试输出）
    // -------------------------------------------------------------------------
    Serial.begin(115200);

    // 等待串口就绪（某些开发板需要）
    delay(2000);

    // 打印启动信息
    Serial.println();
    Serial.println("========================================");
    Serial.println("  Seeed HA Discovery - 温湿度传感器示例");
    Serial.println("========================================");
    Serial.println();

    // -------------------------------------------------------------------------
    // 步骤 2: 初始化 DHT 传感器（如果使用）
    // -------------------------------------------------------------------------
    #ifdef USE_DHT_SENSOR
        Serial.println("正在初始化 DHT22 传感器...");
        dht.begin();
        Serial.println("DHT22 传感器初始化完成");
    #else
        Serial.println("注意: 使用模拟传感器数据");
        Serial.println("如需使用真实 DHT22，请取消代码中的 USE_DHT_SENSOR 注释");
    #endif
    Serial.println();

    // -------------------------------------------------------------------------
    // 步骤 3: 配置设备信息
    // -------------------------------------------------------------------------
    // 这些信息会显示在 Home Assistant 的设备页面
    ha.setDeviceInfo(DEVICE_NAME, DEVICE_MODEL, FIRMWARE_VERSION);

    // 启用调试输出（可选，发布时可关闭）
    ha.enableDebug(true);

    // -------------------------------------------------------------------------
    // 步骤 4: 连接 WiFi 并启动服务
    // -------------------------------------------------------------------------
    Serial.println("正在连接 WiFi...");
    Serial.print("SSID: ");
    Serial.println(WIFI_SSID);

    if (!ha.begin(WIFI_SSID, WIFI_PASSWORD)) {
        // WiFi 连接失败
        Serial.println();
        Serial.println("========================================");
        Serial.println("  WiFi 连接失败！");
        Serial.println("  请检查:");
        Serial.println("  1. WiFi 名称和密码是否正确");
        Serial.println("  2. WiFi 信号是否良好");
        Serial.println("  3. 路由器是否正常工作");
        Serial.println("========================================");

        // 无限循环，等待用户检查
        while (true) {
            delay(1000);
        }
    }

    // -------------------------------------------------------------------------
    // 步骤 5: 创建传感器
    // -------------------------------------------------------------------------
    Serial.println();
    Serial.println("正在创建传感器...");

    // 创建温度传感器
    // 参数说明：
    // - "temperature": 传感器 ID，用于内部识别
    // - "温度": 显示名称，会出现在 Home Assistant UI 中
    // - "temperature": 设备类别，Home Assistant 会自动识别并显示合适的图标
    // - "°C": 单位
    temperatureSensor = ha.addSensor("temperature", "温度", "temperature", "°C");

    // 设置额外属性
    temperatureSensor->setStateClass("measurement");  // 测量类型数据
    temperatureSensor->setPrecision(1);               // 显示 1 位小数
    temperatureSensor->setIcon("mdi:thermometer");    // 自定义图标

    // 创建湿度传感器
    humiditySensor = ha.addSensor("humidity", "湿度", "humidity", "%");
    humiditySensor->setStateClass("measurement");
    humiditySensor->setPrecision(0);                  // 湿度不需要小数
    humiditySensor->setIcon("mdi:water-percent");

    Serial.println("传感器创建完成！");

    // -------------------------------------------------------------------------
    // 步骤 6: 完成初始化
    // -------------------------------------------------------------------------
    Serial.println();
    Serial.println("========================================");
    Serial.println("  初始化完成！");
    Serial.println();
    Serial.print("  设备 IP: ");
    Serial.println(ha.getLocalIP());
    Serial.print("  设备 ID: ");
    Serial.println(ha.getDeviceId());
    Serial.println();
    Serial.println("  在 Home Assistant 中:");
    Serial.println("  1. 进入 设置 -> 设备与服务");
    Serial.println("  2. 点击 添加集成");
    Serial.println("  3. 搜索 Seeed HA Discovery");
    Serial.println("  4. 输入上面的 IP 地址");
    Serial.println();
    Serial.print("  或在浏览器中打开: http://");
    Serial.println(ha.getLocalIP());
    Serial.println("========================================");
    Serial.println();
}

// =============================================================================
// 主循环 - 持续执行
// =============================================================================

void loop() {
    // -------------------------------------------------------------------------
    // 步骤 1: 处理网络事件（必须！）
    // -------------------------------------------------------------------------
    // 这个调用处理所有网络通信，包括：
    // - HTTP 请求
    // - WebSocket 消息
    // - 心跳检测
    // 必须在 loop() 中频繁调用！
    ha.handle();

    // -------------------------------------------------------------------------
    // 步骤 2: 定期读取并上报传感器数据
    // -------------------------------------------------------------------------
    unsigned long currentTime = millis();

    // 检查是否到了更新时间
    if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) {
        lastUpdateTime = currentTime;

        // 读取传感器数据
        float temperature = readTemperature();
        float humidity = readHumidity();

        // 更新传感器值
        // 调用 setValue() 后，新值会自动推送到 Home Assistant
        temperatureSensor->setValue(temperature);
        humiditySensor->setValue(humidity);

        // 在串口输出当前数据（调试用）
        Serial.print("温度: ");
        Serial.print(temperature, 1);
        Serial.print(" °C, 湿度: ");
        Serial.print(humidity, 0);
        Serial.print(" %, Home Assistant: ");
        Serial.println(ha.isHAConnected() ? "已连接" : "未连接");
    }

    // -------------------------------------------------------------------------
    // 步骤 3: 其他任务（可选）
    // -------------------------------------------------------------------------
    // 在这里添加你的其他代码，例如：
    // - 读取其他传感器
    // - 控制 LED
    // - 按键检测
    // 等等...
}
