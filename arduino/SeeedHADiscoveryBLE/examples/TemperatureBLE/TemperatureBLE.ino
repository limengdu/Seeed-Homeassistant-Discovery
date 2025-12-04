/**
 * ============================================================================
 * Seeed HA Discovery BLE - 温湿度传感器示例
 * Temperature & Humidity Sensor Example (BLE)
 * ============================================================================
 *
 * 这个示例展示如何：
 * 1. 通过 BLE 广播温湿度数据到 Home Assistant
 * 2. 使用 BTHome v2 协议实现自动发现
 * 3. 超低功耗运行
 *
 * 硬件要求：
 * - XIAO ESP32-C3/C6/S3 或 XIAO nRF52840
 * - DHT22 温湿度传感器（可选，本示例也可使用模拟数据）
 *
 * DHT22 接线说明：
 * - VCC  → 3.3V
 * - GND  → GND
 * - DATA → D2 (可在下方修改)
 *
 * 软件依赖：
 * - ESP32: NimBLE-Arduino (通过库管理器安装)
 * - nRF52840 mbed: ArduinoBLE (已内置)
 * - nRF52840 Adafruit: Bluefruit (已内置)
 * - DHT sensor library (作者: Adafruit) - 如果使用 DHT22
 *
 * 使用方法：
 * 1. 如果使用 DHT22，取消注释 USE_DHT_SENSOR
 * 2. 上传到 ESP32 或 nRF52840
 * 3. 打开串口监视器查看状态
 * 4. Home Assistant 会自动发现 BTHome 设备
 *
 * @author limengdu
 * @version 1.0.0
 */

#include <SeeedHADiscoveryBLE.h>

// 如果使用 DHT22 传感器，取消下面两行注释
// #include <DHT.h>
// #define USE_DHT_SENSOR

// =============================================================================
// 配置区域 - 请根据你的环境修改
// Configuration - Please modify according to your environment
// =============================================================================

// 设备名称（会显示在 Home Assistant 中）
const char* DEVICE_NAME = "XIAO 温湿度传感器";

// 广播间隔（毫秒）- 越长越省电
const uint32_t ADVERTISE_INTERVAL = 10000;  // 10 秒

// DHT 传感器配置（如果使用）
#ifdef USE_DHT_SENSOR
    #define DHT_PIN D2        // DHT 数据引脚
    #define DHT_TYPE DHT22    // DHT 类型: DHT11 或 DHT22
    DHT dht(DHT_PIN, DHT_TYPE);
#endif

// =============================================================================
// 全局变量
// =============================================================================

SeeedHADiscoveryBLE ble;
SeeedBLESensor* tempSensor;
SeeedBLESensor* humiditySensor;

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
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("========================================");
    Serial.println("  Seeed HA Discovery BLE - 温湿度示例");
    Serial.println("========================================");
    Serial.println();

    // 初始化 DHT 传感器（如果使用）
    #ifdef USE_DHT_SENSOR
        Serial.println("正在初始化 DHT22 传感器...");
        dht.begin();
        Serial.print("DHT22 引脚: D2 (GPIO");
        Serial.print(DHT_PIN);
        Serial.println(")");
    #else
        Serial.println("使用模拟传感器数据");
        Serial.println("如需使用 DHT22，请取消 USE_DHT_SENSOR 注释");
    #endif
    Serial.println();

    // 启用调试输出
    ble.enableDebug(true);

    // 初始化 BLE
    if (!ble.begin(DEVICE_NAME)) {
        Serial.println("BLE 初始化失败！");
        while (1) delay(1000);
    }

    Serial.println("BLE 初始化成功！");

    // 添加温度传感器
    tempSensor = ble.addTemperature();

    // 添加湿度传感器
    humiditySensor = ble.addHumidity();

    Serial.println("传感器已添加: 温度, 湿度");

    Serial.println();
    Serial.println("========================================");
    Serial.println("  初始化完成！");
    Serial.println("========================================");
    Serial.println();
    Serial.println("Home Assistant 会自动发现此设备");
    Serial.println("确保 HA 有蓝牙适配器或蓝牙代理");
    Serial.println();
    Serial.print("设备名称: ");
    Serial.println(DEVICE_NAME);
    Serial.print("MAC 地址: ");
    Serial.println(ble.getAddress());
    Serial.print("广播间隔: ");
    Serial.print(ADVERTISE_INTERVAL / 1000);
    Serial.println(" 秒");
    Serial.println();
    Serial.println("提示: 在 HA 中可通过 MAC 地址识别此设备");
    Serial.println();
}

void loop() {
    // 读取传感器数据
    float temp = readTemperature();
    float humidity = readHumidity();

    // 更新传感器值
    tempSensor->setValue(temp);
    humiditySensor->setValue(humidity);

    // 发送 BLE 广播
    ble.advertise();

    // 打印状态
    Serial.print("广播: 温度=");
    Serial.print(temp, 1);
    Serial.print("C, 湿度=");
    Serial.print(humidity, 0);
    Serial.println("%");

    // 等待下次广播
    delay(ADVERTISE_INTERVAL);
}
