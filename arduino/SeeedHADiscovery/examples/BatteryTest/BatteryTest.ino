/**
 * ============================================================================
 * Battery Voltage Test - 电池电压测试程序
 * ============================================================================
 * 
 * 用于单独测试 IoT Button V2 的电池电压读取功能
 * For testing battery voltage reading on IoT Button V2
 * 
 * Hardware:
 * - ESP32-C6
 * - GPIO0: Battery voltage detection enable (HIGH = enabled)
 * - GPIO1: Battery ADC (12dB attenuation, x3.0 for actual voltage)
 * 
 * @author limengdu
 */

// =============================================================================
// Pin Definitions | 引脚定义
// =============================================================================

#define PIN_BATTERY_EN      0   // Battery voltage detection enable (HIGH = enabled)
#define PIN_BATTERY_ADC     1   // Battery ADC

// =============================================================================
// Battery Configuration | 电池配置
// =============================================================================

#define BATTERY_VOLTAGE_MIN     2.75f  // 0% voltage | 0% 电压
#define BATTERY_VOLTAGE_MAX     4.2f   // 100% voltage | 100% 电压

// ADC 乘数 - 根据实际分压电阻计算
// 如果万用表测量 4.2V，ADC 电压 1.047V，则乘数 = 4.2 / 1.047 ≈ 4.01
#define BATTERY_ADC_MULTIPLIER  4.0f   // ADC to actual voltage multiplier | ADC 到实际电压乘数

// =============================================================================
// Functions | 函数
// =============================================================================

/**
 * Set Battery detection enable | 设置电池检测使能
 */
void setBatteryDetectEnable(bool enable) {
    digitalWrite(PIN_BATTERY_EN, enable ? HIGH : LOW);
}

/**
 * Read battery voltage from ADC | 从 ADC 读取电池电压
 */
float readBatteryVoltage() {
    // Ensure battery detection is enabled | 确保电池检测已使能
    setBatteryDetectEnable(true);
    delay(10);  // Wait for circuit to settle | 等待电路稳定
    
    // Take multiple readings for accuracy | 多次采样提高精度
    uint32_t adcSum = 0;
    for (int i = 0; i < 10; i++) {
        adcSum += analogRead(PIN_BATTERY_ADC);
        delayMicroseconds(100);
    }
    
    float adcAvg = (float)adcSum / 10.0f;
    float adcVoltage = (adcAvg / 4095.0f) * 3.3f;
    float batteryVoltage = adcVoltage * BATTERY_ADC_MULTIPLIER;
    
    // Round to 2 decimal places | 四舍五入到2位小数
    batteryVoltage = round(batteryVoltage * 100.0f) / 100.0f;
    
    return batteryVoltage;
}

/**
 * Calculate battery percentage from voltage
 * 从电压计算电池百分比
 */
float calculateBatteryPercentage(float voltage) {
    if (isnan(voltage) || voltage < BATTERY_VOLTAGE_MIN) {
        return 0.0f;
    }
    if (voltage > BATTERY_VOLTAGE_MAX) {
        return 100.0f;
    }
    
    float percentage = ((voltage - BATTERY_VOLTAGE_MIN) / (BATTERY_VOLTAGE_MAX - BATTERY_VOLTAGE_MIN)) * 100.0f;
    
    // Round to integer | 四舍五入到整数
    percentage = round(percentage);
    
    return constrain(percentage, 0.0f, 100.0f);
}

/**
 * Read and print detailed battery info | 读取并打印详细电池信息
 */
void readBatteryDetailed() {
    // Enable battery detection | 使能电池检测
    setBatteryDetectEnable(true);
    delay(10);
    
    // Read raw ADC values | 读取原始 ADC 值
    uint32_t adcValues[10];
    uint32_t adcSum = 0;
    uint32_t adcMin = 4095;
    uint32_t adcMax = 0;
    
    Serial.println("----------------------------------------");
    Serial.println("Raw ADC Readings (10 samples):");
    
    for (int i = 0; i < 10; i++) {
        adcValues[i] = analogRead(PIN_BATTERY_ADC);
        adcSum += adcValues[i];
        if (adcValues[i] < adcMin) adcMin = adcValues[i];
        if (adcValues[i] > adcMax) adcMax = adcValues[i];
        Serial.printf("  Sample %d: %lu\n", i + 1, adcValues[i]);
        delayMicroseconds(100);
    }
    
    float adcAvg = (float)adcSum / 10.0f;
    
    Serial.println("----------------------------------------");
    Serial.printf("ADC Statistics:\n");
    Serial.printf("  Min: %lu\n", adcMin);
    Serial.printf("  Max: %lu\n", adcMax);
    Serial.printf("  Avg: %.2f\n", adcAvg);
    Serial.printf("  Range: %lu\n", adcMax - adcMin);
    
    // Calculate voltages | 计算电压
    float adcVoltage = (adcAvg / 4095.0f) * 3.3f;
    float batteryVoltage = adcVoltage * BATTERY_ADC_MULTIPLIER;
    float batteryVoltageRounded = round(batteryVoltage * 100.0f) / 100.0f;
    
    Serial.println("----------------------------------------");
    Serial.printf("Voltage Calculation:\n");
    Serial.printf("  ADC Voltage (raw):     %.6f V\n", adcVoltage);
    Serial.printf("  Battery Voltage (raw): %.6f V\n", batteryVoltage);
    Serial.printf("  Battery Voltage (2dp): %.2f V\n", batteryVoltageRounded);
    
    // Calculate percentage | 计算百分比
    float percentageRaw = ((batteryVoltage - BATTERY_VOLTAGE_MIN) / (BATTERY_VOLTAGE_MAX - BATTERY_VOLTAGE_MIN)) * 100.0f;
    float percentageRounded = round(percentageRaw);
    
    Serial.println("----------------------------------------");
    Serial.printf("Percentage Calculation:\n");
    Serial.printf("  Formula: ((%.2f - %.2f) / (%.2f - %.2f)) * 100\n", 
                  batteryVoltage, BATTERY_VOLTAGE_MIN, BATTERY_VOLTAGE_MAX, BATTERY_VOLTAGE_MIN);
    Serial.printf("  Percentage (raw):     %.6f %%\n", percentageRaw);
    Serial.printf("  Percentage (rounded): %.0f %%\n", percentageRounded);
    
    Serial.println("----------------------------------------");
    Serial.printf("==> FINAL: %.2f V, %.0f%%\n", batteryVoltageRounded, percentageRounded);
    Serial.println("========================================\n");
}

// =============================================================================
// Arduino Main Program | Arduino 主程序
// =============================================================================

void setup() {
    // Initialize serial | 初始化串口
    Serial.begin(115200);
    delay(1000);
    
    Serial.println();
    Serial.println("========================================");
    Serial.println("  Battery Voltage Test");
    Serial.println("  电池电压测试程序");
    Serial.println("========================================");
    Serial.println();
    
    // Initialize pins | 初始化引脚
    pinMode(PIN_BATTERY_EN, OUTPUT);
    
    // ADC configuration | ADC 配置
    analogSetAttenuation(ADC_11db);
    
    Serial.println("Configuration:");
    Serial.printf("  Battery Enable Pin: GPIO%d\n", PIN_BATTERY_EN);
    Serial.printf("  Battery ADC Pin:    GPIO%d\n", PIN_BATTERY_ADC);
    Serial.printf("  ADC Multiplier:     %.1f\n", BATTERY_ADC_MULTIPLIER);
    Serial.printf("  Voltage Range:      %.2f - %.2f V\n", BATTERY_VOLTAGE_MIN, BATTERY_VOLTAGE_MAX);
    Serial.println();
    Serial.println("Reading battery every 2 seconds...");
    Serial.println();
}

void loop() {
    // Read detailed battery info | 读取详细电池信息
    readBatteryDetailed();
    
    // Wait 2 seconds | 等待2秒
    delay(2000);
}

