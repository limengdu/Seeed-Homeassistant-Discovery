/**
 * ============================================================================
 * Seeed HA Discovery BLE - LED Switch Example (Bidirectional)
 * Seeed HA Discovery BLE - LED 开关示例（双向通信）
 * ============================================================================
 *
 * This example demonstrates how to:
 * 本示例展示如何：
 * 1. Control LED switch via BLE from Home Assistant
 *    通过 BLE 让 Home Assistant 控制 LED 开关
 * 2. Use GATT service for bidirectional communication
 *    使用 GATT 服务实现双向通信
 * 3. Broadcast BTHome data for sensors
 *    同时广播 BTHome 数据供传感器使用
 *
 * How it works:
 * 工作原理：
 * - Device acts as GATT server, exposes control characteristics
 *   设备作为 GATT 服务器，暴露控制特征值
 * - Home Assistant connects to device and writes commands
 *   Home Assistant 连接设备并写入命令
 * - Device receives command, controls LED, and notifies state change
 *   设备接收命令后控制 LED 并通知状态变化
 *
 * Hardware Requirements:
 * 硬件要求：
 * - XIAO ESP32-C3/C6/S3 or XIAO nRF52840
 * - Onboard LED or external LED
 *   板载 LED 或外接 LED
 *
 * Note:
 * 注意：
 * - XIAO ESP32-C3 has no User LED, external LED required!
 *   XIAO ESP32-C3 没有用户 LED，需要外接！
 * - If using ESP32-C3, uncomment EXTERNAL_LED below
 *   如果使用 ESP32-C3，请取消下方 EXTERNAL_LED 的注释
 *
 * Software Dependencies:
 * 软件依赖：
 * - ESP32: NimBLE-Arduino (install via Library Manager)
 *   ESP32: NimBLE-Arduino（通过库管理器安装）
 * - nRF52840 mbed: ArduinoBLE (built-in)
 *   nRF52840 mbed: ArduinoBLE（已内置）
 *
 * @author limengdu
 * @version 1.5.0
 */

#include <SeeedHADiscoveryBLE.h>

// =============================================================================
// Configuration | 配置区域
// =============================================================================

// Device name (displayed in Home Assistant)
// 设备名称（会显示在 Home Assistant 中）
const char* DEVICE_NAME = "XIAO LED Controller";

// Advertise interval (ms) | 广播间隔（毫秒）
const uint32_t ADVERTISE_INTERVAL = 5000;

// LED Configuration | LED 配置
// If your board has no LED_BUILTIN (like XIAO ESP32-C3), uncomment below
// 如果你的板子没有 LED_BUILTIN（如 XIAO ESP32-C3），取消下面的注释
// #define EXTERNAL_LED
// #define LED_PIN D0  // External LED pin | 外接 LED 引脚

#ifdef EXTERNAL_LED
    #define LED_BUILTIN LED_PIN
#endif

// XIAO series are usually active low
// XIAO 系列通常是低电平点亮
#define LED_ACTIVE_LOW true

// =============================================================================
// Global Variables | 全局变量
// =============================================================================

SeeedHADiscoveryBLE ble;
SeeedBLESwitch* ledSwitch;
SeeedBLESensor* ledStateSensor;  // LED state sensor (for BTHome broadcast) | LED 状态传感器（用于 BTHome 广播）

// =============================================================================
// Helper Functions | 辅助函数
// =============================================================================

/**
 * Set LED state
 * 设置 LED 状态
 */
void setLED(bool on) {
    if (LED_ACTIVE_LOW) {
        digitalWrite(LED_BUILTIN, on ? LOW : HIGH);
    } else {
        digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
    }
}

// =============================================================================
// Arduino Main Program | Arduino 主程序
// =============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("========================================");
    Serial.println("  Seeed HA Discovery BLE - LED Switch");
    Serial.println("  (Bidirectional Version)");
    Serial.println("========================================");
    Serial.println();

    // Initialize LED | 初始化 LED
    pinMode(LED_BUILTIN, OUTPUT);
    setLED(false);  // Initial: OFF | 初始关闭
    Serial.println("LED pin initialized");

    // Enable debug | 启用调试
    ble.enableDebug(true);

    // Initialize BLE (enable control)
    // Second parameter true enables GATT bidirectional communication
    // 初始化 BLE（启用控制功能）
    // 第二个参数 true 表示启用 GATT 双向通信
    if (!ble.begin(DEVICE_NAME, true)) {
        Serial.println("BLE initialization failed!");
        while (1) delay(1000);
    }

    Serial.println("BLE initialization successful!");

    // Add LED state sensor (for BTHome broadcast, so HA can discover device)
    // 添加 LED 状态传感器（用于 BTHome 广播，让 HA 能发现设备）
    ledStateSensor = ble.addSensor(BTHOME_BINARY_POWER);
    ledStateSensor->setState(false);  // Initial state: OFF | 初始状态：关闭

    // Add LED switch | 添加 LED 开关
    ledSwitch = ble.addSwitch("led", "Onboard LED");

    // Register callback: executed when HA sends control command
    // 注册回调：当 HA 发送控制命令时执行
    ledSwitch->onStateChange([](bool state) {
        setLED(state);
        ledStateSensor->setState(state);  // Sync sensor state | 同步更新传感器状态
        Serial.print("HA Command: LED -> ");
        Serial.println(state ? "ON" : "OFF");
    });

    Serial.println("LED switch and state sensor added");

    // Start advertising | 开始广播
    ble.advertise();

    Serial.println();
    Serial.println("========================================");
    Serial.println("  Initialization Complete!");
    Serial.println("========================================");
    Serial.println();
    Serial.print("Device Name: ");
    Serial.println(DEVICE_NAME);
    Serial.print("MAC Address: ");
    Serial.println(ble.getAddress());
    Serial.print("Advertise Interval: ");
    Serial.print(ADVERTISE_INTERVAL / 1000);
    Serial.println(" seconds");
    Serial.println();
    Serial.println("GATT Service UUIDs:");
    Serial.print("  Control Service: ");
    Serial.println(SEEED_CONTROL_SERVICE_UUID);
    Serial.print("  Command Char: ");
    Serial.println(SEEED_CONTROL_COMMAND_CHAR_UUID);
    Serial.print("  State Char: ");
    Serial.println(SEEED_CONTROL_STATE_CHAR_UUID);
    Serial.println();
    Serial.println("Waiting for Home Assistant connection...");
    Serial.println("Tip: Use nRF Connect or other BLE tools to test");
    Serial.println();
    Serial.println("Command format: [switch_index][state]");
    Serial.println("  e.g.: 0x00 0x01 = Switch 0 ON");
    Serial.println("        0x00 0x00 = Switch 0 OFF");
    Serial.println();
}

void loop() {
    // Handle BLE events (must call!)
    // 处理 BLE 事件（必须调用！）
    ble.loop();

    // Periodically advertise BTHome data
    // 定期广播 BTHome 数据
    static unsigned long lastAdvertise = 0;
    if (millis() - lastAdvertise >= ADVERTISE_INTERVAL) {
        lastAdvertise = millis();
        ble.advertise();

        // Print status | 打印状态
        Serial.print("Status: LED=");
        Serial.print(ledSwitch->getState() ? "ON" : "OFF");
        Serial.print(", Connected=");
        Serial.println(ble.isConnected() ? "Yes" : "No");
    }
}
