/**
 * ============================================================================
 * Seeed HA Discovery BLE - Button Example
 * Seeed HA Discovery BLE - 按钮示例
 * ============================================================================
 *
 * This example demonstrates how to:
 * 本示例展示如何：
 * 1. Send button events to Home Assistant via BLE
 *    通过 BLE 发送按钮事件到 Home Assistant
 * 2. Support single click, double click, long press events
 *    支持单击、双击、长按等事件
 * 3. Ultra-low power design
 *    超低功耗设计
 *
 * Hardware Requirements:
 * 硬件要求：
 * - XIAO ESP32-C3/C6/S3 or XIAO nRF52840
 * - Button connected to specified pin (default D1)
 *   按钮接在指定引脚（默认 D1）
 *
 * Software Dependencies:
 * 软件依赖：
 * - ESP32: NimBLE-Arduino (install via Library Manager)
 *   ESP32: NimBLE-Arduino（通过库管理器安装）
 * - nRF52840 mbed: ArduinoBLE (built-in)
 *   nRF52840 mbed: ArduinoBLE（已内置）
 * - nRF52840 Adafruit: Bluefruit (built-in)
 *   nRF52840 Adafruit: Bluefruit（已内置）
 *
 * @author limengdu
 * @version 1.5.0
 */

#include <SeeedHADiscoveryBLE.h>

// =============================================================================
// Configuration | 配置区域
// =============================================================================

// Device name | 设备名称
const char* DEVICE_NAME = "XIAO Button";

// Button pin | 按钮引脚
#define BUTTON_PIN D1

// Long press threshold (ms) | 长按阈值（毫秒）
#define LONG_PRESS_TIME 1000

// Double click interval (ms) | 双击间隔（毫秒）
#define DOUBLE_CLICK_TIME 300

// =============================================================================
// Global Variables | 全局变量
// =============================================================================

SeeedHADiscoveryBLE ble;

// Three switches for three press types
// 三个开关，对应三种按法
SeeedBLESwitch* singleClickSwitch;
SeeedBLESwitch* doubleClickSwitch;
SeeedBLESwitch* longPressSwitch;

// State sensors (for BTHome broadcast)
// 状态传感器（用于BTHome广播）
SeeedBLESensor* singleState;
SeeedBLESensor* doubleState;
SeeedBLESensor* longState;

// Button state | 按钮状态
bool lastButtonState = HIGH;
unsigned long buttonPressTime = 0;
unsigned long lastClickTime = 0;
uint8_t clickCount = 0;

// Advertise interval | 广播间隔
unsigned long lastAdvertise = 0;
#define ADVERTISE_INTERVAL 5000  // Advertise every 5 seconds (for discovery) | 每5秒广播一次（用于发现）

// =============================================================================
// Button Handling | 按钮处理
// =============================================================================

BTHomeButtonEvent detectButtonEvent() {
    bool currentState = digitalRead(BUTTON_PIN);
    BTHomeButtonEvent event = BTHOME_BUTTON_NONE;
    unsigned long now = millis();

    // Detect press | 检测按下
    if (lastButtonState == HIGH && currentState == LOW) {
        buttonPressTime = now;
    }

    // Detect release | 检测释放
    if (lastButtonState == LOW && currentState == HIGH) {
        unsigned long pressDuration = now - buttonPressTime;

        if (pressDuration >= LONG_PRESS_TIME) {
            // Long press | 长按
            event = BTHOME_BUTTON_LONG_PRESS;
            clickCount = 0;
        } else {
            // Short press, detect double/triple click
            // 短按，检测双击/三击
            if (now - lastClickTime < DOUBLE_CLICK_TIME) {
                clickCount++;
            } else {
                clickCount = 1;
            }
            lastClickTime = now;
        }
    }

    // Detect double/triple click timeout
    // 检测双击/三击超时
    if (clickCount > 0 && now - lastClickTime > DOUBLE_CLICK_TIME) {
        switch (clickCount) {
            case 1:
                event = BTHOME_BUTTON_PRESS;
                break;
            case 2:
                event = BTHOME_BUTTON_DOUBLE;
                break;
            case 3:
            default:
                event = BTHOME_BUTTON_TRIPLE;
                break;
        }
        clickCount = 0;
    }

    lastButtonState = currentState;
    return event;
}

// =============================================================================
// Arduino Main Program | Arduino 主程序
// =============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("========================================");
    Serial.println("  Seeed HA Discovery BLE - Button");
    Serial.println("========================================");
    Serial.println();

    // Initialize button pin | 初始化按钮引脚
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    Serial.print("Button Pin: D1 (GPIO");
    Serial.print(BUTTON_PIN);
    Serial.println(")");

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

    // Add state sensors (for BTHome broadcast, so HA can display state)
    // 添加状态传感器（用于BTHome广播，让HA显示状态）
    singleState = ble.addSensor(BTHOME_BINARY_POWER);
    singleState->setState(false);
    doubleState = ble.addSensor(BTHOME_BINARY_GENERIC);
    doubleState->setState(false);
    longState = ble.addSensor(BTHOME_BINARY_OPENING);  // Use different type to distinguish | 使用不同的类型区分
    longState->setState(false);

    // Add three switches for three press types (GATT control)
    // 添加三个开关，对应三种按法（GATT控制）
    singleClickSwitch = ble.addSwitch("single", "Single Click");
    doubleClickSwitch = ble.addSwitch("double", "Double Click");
    longPressSwitch = ble.addSwitch("long", "Long Press");

    // Register callbacks: sync sensor state when HA remotely controls
    // 注册回调：当 HA 远程控制时，同步更新传感器状态
    singleClickSwitch->onStateChange([](bool state) {
        singleState->setState(state);
        Serial.print("HA Control [Single Click]: ");
        Serial.println(state ? "ON" : "OFF");
    });
    doubleClickSwitch->onStateChange([](bool state) {
        doubleState->setState(state);
        Serial.print("HA Control [Double Click]: ");
        Serial.println(state ? "ON" : "OFF");
    });
    longPressSwitch->onStateChange([](bool state) {
        longState->setState(state);
        Serial.print("HA Control [Long Press]: ");
        Serial.println(state ? "ON" : "OFF");
    });

    Serial.println("Three switches added");

    Serial.println();
    Serial.println("========================================");
    Serial.println("  Initialization Complete!");
    Serial.println("========================================");
    Serial.println();
    Serial.print("Device Name: ");
    Serial.println(DEVICE_NAME);
    Serial.print("MAC Address: ");
    Serial.println(ble.getAddress());
    Serial.println();
    Serial.println("Tip: Use MAC address to identify this device in HA");
    Serial.println();
    Serial.println("Supported button events:");
    Serial.println("  - Single click");
    Serial.println("  - Double click");
    Serial.println("  - Triple click");
    Serial.println("  - Long press (>1s)");
    Serial.println();
    Serial.println("Waiting for button events...");
    Serial.println("Button press will toggle corresponding switch state");
    Serial.println();
}

void setLED(bool state) {
    // LED feedback (if available) | LED 反馈（如果有）
    // digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
}

void loop() {
    unsigned long now = millis();

    // Handle BLE GATT events (must call)
    // 处理 BLE GATT 事件（必须调用）
    ble.loop();

    // Detect button event | 检测按钮事件
    BTHomeButtonEvent event = detectButtonEvent();

    // If event detected, toggle corresponding switch state
    // 如果有事件，切换对应开关的状态
    if (event != BTHOME_BUTTON_NONE) {
        const char* eventName = "Unknown";
        SeeedBLESwitch* targetSwitch = nullptr;

        switch (event) {
            case BTHOME_BUTTON_PRESS:
                eventName = "Single Click";
                targetSwitch = singleClickSwitch;
                break;
            case BTHOME_BUTTON_DOUBLE:
                eventName = "Double Click";
                targetSwitch = doubleClickSwitch;
                break;
            case BTHOME_BUTTON_TRIPLE:
                // Triple click treated as double click
                // 三击作为双击处理
                eventName = "Triple Click (as Double)";
                targetSwitch = doubleClickSwitch;
                break;
            case BTHOME_BUTTON_LONG_PRESS:
                eventName = "Long Press";
                targetSwitch = longPressSwitch;
                break;
            default:
                break;
        }

        if (targetSwitch) {
            // Toggle switch state | 切换开关状态
            bool newState = !targetSwitch->getState();
            targetSwitch->setState(newState);

            // Sync corresponding sensor state
            // 同步更新对应的传感器状态
            if (targetSwitch == singleClickSwitch) {
                singleState->setState(newState);
            } else if (targetSwitch == doubleClickSwitch) {
                doubleState->setState(newState);
            } else if (targetSwitch == longPressSwitch) {
                longState->setState(newState);
            }

            Serial.print("Button Event: ");
            Serial.print(eventName);
            Serial.print(" -> Switch State: ");
            Serial.println(newState ? "ON" : "OFF");

            // Immediately advertise state update
            // 立即广播状态更新
            ble.advertise();
            lastAdvertise = now;
        }
    }

    // Periodic advertise to keep device discoverable
    // 定期广播，保持设备可被发现
    if (now - lastAdvertise >= ADVERTISE_INTERVAL) {
        ble.advertise();
        lastAdvertise = now;
    }

    delay(10);  // Button debounce | 按钮去抖
}
