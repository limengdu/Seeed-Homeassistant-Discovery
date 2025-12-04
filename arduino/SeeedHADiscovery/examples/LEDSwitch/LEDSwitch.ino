/**
 * ============================================================================
 * Seeed HA Discovery - LED 开关示例
 * LED Switch Example
 * ============================================================================
 *
 * 这个示例展示如何：
 * 1. 创建一个开关实体控制 LED
 * 2. 接收来自 Home Assistant 的开关命令
 * 3. 在 HA 界面实时控制 LED 亮灭
 *
 * ⚠️ 重要提示 (Tips):
 * - XIAO ESP32-C3 没有用户 LED (User LED)，需要外接 LED
 * - XIAO ESP32-S3 的用户 LED 在 GPIO21
 * - XIAO ESP32-C6 的用户 LED 在 GPIO15
 * - 如果你的开发板有板载 LED，请根据实际情况修改 LED_PIN
 *
 * 外接 LED 接线方法：
 * - LED 正极 (长脚) → GPIO (通过 220Ω 电阻)
 * - LED 负极 (短脚) → GND
 *
 * 硬件要求：
 * - XIAO ESP32-C3/C6/S3 或其他 ESP32 开发板
 * - LED + 220Ω 电阻（如果需要外接）
 *
 * 软件依赖：
 * - ArduinoJson (作者: Benoit Blanchon)
 * - WebSockets (作者: Markus Sattler)
 *
 * 使用方法：
 * 1. 修改下方的 WiFi 配置和 LED 引脚
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
const char* WIFI_SSID = "你的WiFi名称";      // Your WiFi SSID
const char* WIFI_PASSWORD = "你的WiFi密码";  // Your WiFi password

// =============================================================================
// LED 引脚配置
// LED Pin Configuration
// =============================================================================

// 使用 LED_BUILTIN 宏最大化兼容性
// 大多数开发板都定义了这个宏指向板载 LED

// ⚠️ 注意：XIAO ESP32-C3 没有用户 LED，需要外接！
// 如果你使用 XIAO ESP32-C3，请取消下面的注释并连接外部 LED：
// #undef LED_BUILTIN
// #define LED_BUILTIN D0  // 外接 LED 的引脚

// LED 极性配置
// true  = 低电平点亮 (XIAO 系列都是低电平点亮)
// false = 高电平点亮 (外接 LED 通常是高电平点亮)
#define LED_ACTIVE_LOW true  // XIAO 默认低电平亮灯

// =============================================================================
// 全局变量
// =============================================================================

SeeedHADiscovery ha;
SeeedHASwitch* ledSwitch;

// =============================================================================
// 辅助函数
// =============================================================================

/**
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
// Arduino 主程序
// =============================================================================

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("========================================");
    Serial.println("  Seeed HA Discovery - LED 开关示例");
    Serial.println("========================================");
    Serial.println();

    // 初始化 LED 引脚
    pinMode(LED_BUILTIN, OUTPUT);
    setLED(false);  // 初始状态为关闭

    Serial.printf("LED 引脚: GPIO%d\n", LED_BUILTIN);
    Serial.printf("LED 极性: %s\n", LED_ACTIVE_LOW ? "低电平点亮" : "高电平点亮");

    // 配置设备信息
    ha.setDeviceInfo(
        "LED 控制器",        // 设备名称
        "XIAO ESP32",        // 设备型号
        "1.0.0"              // 固件版本
    );

    ha.enableDebug(true);

    // 连接 WiFi
    Serial.println("正在连接 WiFi...");

    if (!ha.begin(WIFI_SSID, WIFI_PASSWORD)) {
        Serial.println("❌ WiFi 连接失败！");
        while (1) {
            setLED(true);
            delay(200);
            setLED(false);
            delay(200);
        }
    }

    Serial.println("✅ WiFi 连接成功！");
    Serial.printf("IP 地址: %s\n", ha.getLocalIP().toString().c_str());

    // =========================================================================
    // 创建 LED 开关
    // =========================================================================

    ledSwitch = ha.addSwitch("led", "LED", "mdi:led-on");

    // 注册回调 - 当 HA 发送命令时执行
    ledSwitch->onStateChange([](bool state) {
        Serial.printf("收到命令: %s\n", state ? "开启" : "关闭");
        setLED(state);
        Serial.printf("LED 已%s\n", state ? "点亮" : "熄灭");
    });

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
