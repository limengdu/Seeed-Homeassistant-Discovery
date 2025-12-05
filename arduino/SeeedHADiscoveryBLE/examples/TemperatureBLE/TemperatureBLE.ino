/**
 * ============================================================================
 * Seeed HA Discovery BLE - Temperature & Humidity Sensor Example
 * Seeed HA Discovery BLE - 温湿度传感器示例
 * ============================================================================
 *
 * This example demonstrates how to:
 * 本示例展示如何：
 * 1. Broadcast temperature/humidity data to Home Assistant via BLE
 *    通过 BLE 广播温湿度数据到 Home Assistant
 * 2. Use BTHome v2 protocol for automatic discovery
 *    使用 BTHome v2 协议实现自动发现
 * 3. Ultra-low power operation
 *    超低功耗运行
 *
 * Hardware Requirements:
 * 硬件要求：
 * - XIAO ESP32-C3/C6/S3 or XIAO nRF52840
 * - DHT22 temperature/humidity sensor (optional, can use simulated data)
 *   DHT22 温湿度传感器（可选，本示例也可使用模拟数据）
 *
 * DHT22 Wiring:
 * DHT22 接线说明：
 * - VCC  -> 3.3V
 * - GND  -> GND
 * - DATA -> D2 (configurable below)
 *   DATA -> D2 (可在下方修改)
 *
 * Software Dependencies:
 * 软件依赖：
 * - ESP32: NimBLE-Arduino (install via Library Manager)
 *   ESP32: NimBLE-Arduino（通过库管理器安装）
 * - nRF52840 mbed: ArduinoBLE (built-in)
 *   nRF52840 mbed: ArduinoBLE（已内置）
 * - nRF52840 Adafruit: Bluefruit (built-in)
 *   nRF52840 Adafruit: Bluefruit（已内置）
 * - DHT sensor library (by Adafruit) - if using DHT22
 *   DHT sensor library（作者: Adafruit）- 如果使用 DHT22
 *
 * Usage:
 * 使用方法：
 * 1. If using DHT22, uncomment USE_DHT_SENSOR
 *    如果使用 DHT22，取消注释 USE_DHT_SENSOR
 * 2. Upload to ESP32 or nRF52840
 *    上传到 ESP32 或 nRF52840
 * 3. Open Serial Monitor to view status
 *    打开串口监视器查看状态
 * 4. Home Assistant will automatically discover BTHome device
 *    Home Assistant 会自动发现 BTHome 设备
 *
 * @author limengdu
 * @version 1.5.0
 */

#include <SeeedHADiscoveryBLE.h>

// If using DHT22 sensor, uncomment the following two lines
// 如果使用 DHT22 传感器，取消下面两行注释
// #include <DHT.h>
// #define USE_DHT_SENSOR

// =============================================================================
// Configuration - Please modify according to your environment
// 配置区域 - 请根据你的环境修改
// =============================================================================

// Device name (displayed in Home Assistant)
// 设备名称（会显示在 Home Assistant 中）
const char* DEVICE_NAME = "XIAO Temp/Humidity";

// Advertise interval (ms) - longer = more power saving
// 广播间隔（毫秒）- 越长越省电
const uint32_t ADVERTISE_INTERVAL = 10000;  // 10 seconds | 10 秒

// DHT Sensor Configuration (if using)
// DHT 传感器配置（如果使用）
#ifdef USE_DHT_SENSOR
    #define DHT_PIN D2        // DHT data pin | DHT 数据引脚
    #define DHT_TYPE DHT22    // DHT type: DHT11 or DHT22 | DHT 类型
    DHT dht(DHT_PIN, DHT_TYPE);
#endif

// =============================================================================
// Global Variables | 全局变量
// =============================================================================

SeeedHADiscoveryBLE ble;
SeeedBLESensor* tempSensor;
SeeedBLESensor* humiditySensor;

// =============================================================================
// Helper Functions | 辅助函数
// =============================================================================

/**
 * Read temperature value
 * 读取温度值
 */
float readTemperature() {
    #ifdef USE_DHT_SENSOR
        float temp = dht.readTemperature();
        if (isnan(temp)) {
            Serial.println("DHT22 temperature read failed!");
            return 0;
        }
        return temp;
    #else
        // Simulated data: fluctuates between 20-30°C
        // 模拟数据：20-30°C 之间波动
        static float baseTemp = 25.0;
        baseTemp += (random(-10, 11)) / 100.0;
        if (baseTemp < 20) baseTemp = 20;
        if (baseTemp > 30) baseTemp = 30;
        return baseTemp;
    #endif
}

/**
 * Read humidity value
 * 读取湿度值
 */
float readHumidity() {
    #ifdef USE_DHT_SENSOR
        float humidity = dht.readHumidity();
        if (isnan(humidity)) {
            Serial.println("DHT22 humidity read failed!");
            return 0;
        }
        return humidity;
    #else
        // Simulated data: fluctuates between 40-70%
        // 模拟数据：40-70% 之间波动
        static float baseHumidity = 55.0;
        baseHumidity += (random(-10, 11)) / 50.0;
        if (baseHumidity < 40) baseHumidity = 40;
        if (baseHumidity > 70) baseHumidity = 70;
        return baseHumidity;
    #endif
}

// =============================================================================
// Arduino Main Program | Arduino 主程序
// =============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("========================================");
    Serial.println("  Seeed HA Discovery BLE - Temp/Humidity");
    Serial.println("========================================");
    Serial.println();

    // Initialize DHT sensor (if using)
    // 初始化 DHT 传感器（如果使用）
    #ifdef USE_DHT_SENSOR
        Serial.println("Initializing DHT22 sensor...");
        dht.begin();
        Serial.print("DHT22 Pin: D2 (GPIO");
        Serial.print(DHT_PIN);
        Serial.println(")");
    #else
        Serial.println("Using simulated sensor data");
        Serial.println("To use DHT22, uncomment USE_DHT_SENSOR");
    #endif
    Serial.println();

    // Enable debug | 启用调试输出
    ble.enableDebug(true);

    // Initialize BLE | 初始化 BLE
    if (!ble.begin(DEVICE_NAME)) {
        Serial.println("BLE initialization failed!");
        while (1) delay(1000);
    }

    Serial.println("BLE initialization successful!");

    // Add temperature sensor | 添加温度传感器
    tempSensor = ble.addTemperature();

    // Add humidity sensor | 添加湿度传感器
    humiditySensor = ble.addHumidity();

    Serial.println("Sensors added: Temperature, Humidity");

    Serial.println();
    Serial.println("========================================");
    Serial.println("  Initialization Complete!");
    Serial.println("========================================");
    Serial.println();
    Serial.println("Home Assistant will automatically discover this device");
    Serial.println("Make sure HA has Bluetooth adapter or Bluetooth proxy");
    Serial.println();
    Serial.print("Device Name: ");
    Serial.println(DEVICE_NAME);
    Serial.print("MAC Address: ");
    Serial.println(ble.getAddress());
    Serial.print("Advertise Interval: ");
    Serial.print(ADVERTISE_INTERVAL / 1000);
    Serial.println(" seconds");
    Serial.println();
    Serial.println("Tip: Use MAC address to identify this device in HA");
    Serial.println();
}

void loop() {
    // Read sensor data | 读取传感器数据
    float temp = readTemperature();
    float humidity = readHumidity();

    // Update sensor values | 更新传感器值
    tempSensor->setValue(temp);
    humiditySensor->setValue(humidity);

    // Send BLE broadcast | 发送 BLE 广播
    ble.advertise();

    // Print status | 打印状态
    Serial.print("Broadcast: Temp=");
    Serial.print(temp, 1);
    Serial.print("C, Humidity=");
    Serial.print(humidity, 0);
    Serial.println("%");

    // Wait for next broadcast | 等待下次广播
    delay(ADVERTISE_INTERVAL);
}
