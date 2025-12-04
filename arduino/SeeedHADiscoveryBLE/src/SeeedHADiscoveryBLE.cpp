/**
 * ============================================================================
 * Seeed Home Assistant Discovery BLE - 实现文件
 * Seeed Home Assistant Discovery BLE - Implementation
 * ============================================================================
 *
 * 支持平台 | Supported Platforms:
 * - ESP32 系列 (NimBLE)
 * - XIAO nRF52840 mbed 版本 (ArduinoBLE)
 * - XIAO nRF52840 Adafruit BSP (Bluefruit)
 *
 * 参考 | References:
 * - https://wiki.seeedstudio.com/XIAO-BLE-Sense-Bluetooth-Usage/ (mbed + ArduinoBLE)
 * - https://wiki.seeedstudio.com/XIAO-BLE-Sense-Bluetooth_Usage/ (Adafruit + Bluefruit)
 * - https://bthome.io/format/ (BTHome v2 规范)
 *
 * @author limengdu
 */

#include "SeeedHADiscoveryBLE.h"
#include <stdarg.h>

// =============================================================================
// SeeedBLESensor 实现 | SeeedBLESensor Implementation
// =============================================================================

SeeedBLESensor::SeeedBLESensor(BTHomeObjectId objectId)
    : _objectId(objectId)
    , _rawValue(0)
    , _hasValue(false)
{
}

void SeeedBLESensor::setValue(int32_t value) {
    _rawValue = value;
    _hasValue = true;
}

void SeeedBLESensor::setValue(float value) {
    // 根据对象类型转换浮点数到整数
    float multiplier = _getMultiplier();
    _rawValue = (int32_t)(value * multiplier);
    _hasValue = true;
}

void SeeedBLESensor::setState(bool state) {
    _rawValue = state ? 1 : 0;
    _hasValue = true;
}

void SeeedBLESensor::triggerButton(BTHomeButtonEvent event) {
    _rawValue = event;
    _hasValue = true;
}

float SeeedBLESensor::_getMultiplier() const {
    switch (_objectId) {
        case BTHOME_TEMPERATURE:
        case BTHOME_HUMIDITY:
        case BTHOME_DEWPOINT:
        case BTHOME_MOISTURE:
        case BTHOME_SPEED:
            return 100.0f;  // 0.01 精度

        case BTHOME_PRESSURE:
        case BTHOME_ILLUMINANCE:
        case BTHOME_POWER:
            return 100.0f;  // 0.01 精度

        case BTHOME_VOLTAGE:
        case BTHOME_CURRENT:
        case BTHOME_ENERGY:
        case BTHOME_GAS:
        case BTHOME_VOLUME_FLOW:
        case BTHOME_WATER:
        case BTHOME_VOLUME_UINT32:
        case BTHOME_GAS_UINT32:
        case BTHOME_ENERGY_UINT32:
        case BTHOME_DURATION:
            return 1000.0f;  // 0.001 精度

        case BTHOME_MASS_KG:
        case BTHOME_MASS_LB:
            return 100.0f;  // 0.01 精度

        case BTHOME_TEMPERATURE_TENTH:
        case BTHOME_ROTATION:
        case BTHOME_DISTANCE_M:
        case BTHOME_VOLUME_LITERS:
        case BTHOME_VOLTAGE_TENTH:
            return 10.0f;  // 0.1 精度

        default:
            return 1.0f;  // 整数
    }
}

uint8_t SeeedBLESensor::getDataSize() const {
    switch (_objectId) {
        // 1 字节数据
        case BTHOME_BATTERY:
        case BTHOME_COUNT_UINT8:
        case BTHOME_HUMIDITY_UINT8:
        case BTHOME_MOISTURE_UINT8:
        case BTHOME_UV_INDEX:
        case BTHOME_BINARY_GENERIC:
        case BTHOME_BINARY_POWER:
        case BTHOME_BINARY_OPENING:
        case BTHOME_BINARY_BATTERY_LOW:
        case BTHOME_BINARY_BATTERY_CHARGING:
        case BTHOME_BINARY_MOTION:
        case BTHOME_BINARY_OCCUPANCY:
        case BTHOME_BUTTON:
            return 1;

        // 2 字节数据
        case BTHOME_TEMPERATURE:
        case BTHOME_HUMIDITY:
        case BTHOME_DEWPOINT:
        case BTHOME_MASS_KG:
        case BTHOME_MASS_LB:
        case BTHOME_COUNT_UINT16:
        case BTHOME_VOLTAGE:
        case BTHOME_PM25:
        case BTHOME_PM10:
        case BTHOME_CO2:
        case BTHOME_TVOC:
        case BTHOME_MOISTURE:
        case BTHOME_DISTANCE_MM:
        case BTHOME_DISTANCE_M:
        case BTHOME_CURRENT:
        case BTHOME_SPEED:
        case BTHOME_TEMPERATURE_TENTH:
        case BTHOME_VOLUME_LITERS:
        case BTHOME_VOLUME_ML:
        case BTHOME_VOLUME_FLOW:
        case BTHOME_VOLTAGE_TENTH:
        case BTHOME_ROTATION:
            return 2;

        // 3 字节数据
        case BTHOME_PRESSURE:
        case BTHOME_ILLUMINANCE:
        case BTHOME_ENERGY:
        case BTHOME_POWER:
        case BTHOME_DURATION:
        case BTHOME_GAS:
            return 3;

        // 4 字节数据
        case BTHOME_COUNT_UINT32:
        case BTHOME_GAS_UINT32:
        case BTHOME_ENERGY_UINT32:
        case BTHOME_VOLUME_UINT32:
        case BTHOME_WATER:
            return 4;

        default:
            return 2;  // 默认 2 字节
    }
}

void SeeedBLESensor::writeToBuffer(uint8_t* buffer, uint8_t& offset) const {
    if (!_hasValue) return;

    // 写入 Object ID
    buffer[offset++] = _objectId;

    // 写入数据（小端序）
    uint8_t dataSize = getDataSize();
    for (uint8_t i = 0; i < dataSize; i++) {
        buffer[offset++] = (_rawValue >> (i * 8)) & 0xFF;
    }
}

// =============================================================================
// SeeedHADiscoveryBLE 实现 | SeeedHADiscoveryBLE Implementation
// =============================================================================

SeeedHADiscoveryBLE::SeeedHADiscoveryBLE()
    : _debug(false)
    , _running(false)
    , _advertiseInterval(5000)
    , _txPower(0)
    , _packetId(0)
    , _advDataLen(0)
#ifdef SEEED_BLE_ESP32
    , _pAdvertising(nullptr)
#endif
{
    strcpy(_deviceName, "Seeed Sensor");
    memset(_advData, 0, sizeof(_advData));
}

SeeedHADiscoveryBLE::~SeeedHADiscoveryBLE() {
    stop();
    for (auto sensor : _sensors) {
        delete sensor;
    }
    _sensors.clear();
}

void SeeedHADiscoveryBLE::setDeviceName(const char* name) {
    strncpy(_deviceName, name, 20);
    _deviceName[20] = '\0';
}

void SeeedHADiscoveryBLE::enableDebug(bool enable) {
    _debug = enable;
}

void SeeedHADiscoveryBLE::setAdvertiseInterval(uint32_t intervalMs) {
    _advertiseInterval = intervalMs;
}

void SeeedHADiscoveryBLE::setTxPower(int8_t power) {
    _txPower = power;
}

bool SeeedHADiscoveryBLE::begin(const char* deviceName) {
    if (deviceName) {
        setDeviceName(deviceName);
    }

    _log("====================================");
    _log("Seeed HA Discovery BLE");
    _log("====================================");

// =============================================================================
// ESP32 平台 - 使用 NimBLE
// =============================================================================
#ifdef SEEED_BLE_ESP32
    _log(SEEED_BLE_PLATFORM);

    NimBLEDevice::init(_deviceName);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);  // 最大功率

    _pAdvertising = NimBLEDevice::getAdvertising();

    _running = true;
    _log("BLE OK");
    return true;

// =============================================================================
// nRF52840 mbed 平台 - 使用 ArduinoBLE
// =============================================================================
#elif defined(SEEED_BLE_MBED_NRF52840)
    _log(SEEED_BLE_PLATFORM);

    // 初始化 BLE
    if (!BLE.begin()) {
        _log("BLE init failed!");
        return false;
    }

    // 设置设备名称
    BLE.setLocalName(_deviceName);
    BLE.setDeviceName(_deviceName);

    _running = true;
    _log("BLE OK");
    return true;

// =============================================================================
// nRF52840 Adafruit BSP - 使用 Bluefruit
// =============================================================================
#elif defined(SEEED_BLE_NRF52_BLUEFRUIT)
    _log(SEEED_BLE_PLATFORM);

    Bluefruit.begin();
    Bluefruit.setTxPower(4);  // 4 dBm
    Bluefruit.setName(_deviceName);

    // 设置广播参数
    Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED);
    Bluefruit.Advertising.clearData();

    _running = true;
    _log("BLE OK");
    return true;

#else
    _log("Unsupported platform");
    return false;
#endif
}

void SeeedHADiscoveryBLE::stop() {
    if (!_running) return;

#ifdef SEEED_BLE_ESP32
    NimBLEDevice::deinit();

#elif defined(SEEED_BLE_MBED_NRF52840)
    BLE.stopAdvertise();
    BLE.end();

#elif defined(SEEED_BLE_NRF52_BLUEFRUIT)
    Bluefruit.Advertising.stop();
#endif

    _running = false;
    _log("BLE stopped");
}

String SeeedHADiscoveryBLE::getAddress() {
    if (!_running) {
        return "00:00:00:00:00:00";
    }

#ifdef SEEED_BLE_ESP32
    // ESP32 NimBLE: 获取 MAC 地址
    NimBLEAddress addr = NimBLEDevice::getAddress();
    return String(addr.toString().c_str());

#elif defined(SEEED_BLE_MBED_NRF52840)
    // nRF52840 ArduinoBLE: 获取 MAC 地址
    return BLE.address();

#elif defined(SEEED_BLE_NRF52_BLUEFRUIT)
    // nRF52840 Bluefruit: 获取 MAC 地址
    uint8_t mac[6];
    Bluefruit.getAddr(mac);
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
    return String(macStr);

#else
    return "00:00:00:00:00:00";
#endif
}

SeeedBLESensor* SeeedHADiscoveryBLE::addSensor(BTHomeObjectId objectId) {
    SeeedBLESensor* sensor = new SeeedBLESensor(objectId);
    _sensors.push_back(sensor);

    if (_debug) {
        Serial.print("[SeeedBLE] Add sensor: 0x");
        Serial.println(objectId, HEX);
    }
    return sensor;
}

void SeeedHADiscoveryBLE::_buildAdvData() {
    _advDataLen = 0;

    // BTHome Service Data 格式:
    // [长度][类型=0x16][UUID低][UUID高][设备信息][传感器数据...]

    // 首先计算传感器数据长度
    uint8_t sensorDataLen = 0;
    for (auto sensor : _sensors) {
        if (sensor->hasValue()) {
            sensorDataLen += 1 + sensor->getDataSize();  // Object ID + Data
        }
    }

    // Service Data 总长度 = UUID(2) + 设备信息(1) + 传感器数据
    uint8_t serviceDataLen = 2 + 1 + sensorDataLen;

    // 构建广播数据
    uint8_t offset = 0;

    // 1. Flags
    _advData[offset++] = 0x02;  // 长度
    _advData[offset++] = 0x01;  // AD Type: Flags
    _advData[offset++] = 0x06;  // LE General Discoverable + BR/EDR Not Supported

    // 2. Service Data (BTHome)
    _advData[offset++] = serviceDataLen + 1;  // 长度（包括类型字节）
    _advData[offset++] = 0x16;  // AD Type: Service Data - 16 bit UUID

    // BTHome Service UUID (0xFCD2, 小端序)
    _advData[offset++] = 0xD2;
    _advData[offset++] = 0xFC;

    // 设备信息字节
    // Bit 0: 加密 (0 = 无加密)
    // Bit 2: 触发器 (0 = 非触发器)
    // Bit 5-7: 版本 (0x02 = BTHome v2)
    _advData[offset++] = BTHOME_DEVICE_INFO_VERSION;  // 0x40 = BTHome v2

    // 传感器数据
    for (auto sensor : _sensors) {
        if (sensor->hasValue()) {
            sensor->writeToBuffer(_advData, offset);
        }
    }

    _advDataLen = offset;
}

void SeeedHADiscoveryBLE::updateAdvertiseData() {
    _buildAdvData();

// =============================================================================
// ESP32 - NimBLE
// =============================================================================
#ifdef SEEED_BLE_ESP32
    if (_pAdvertising) {
        _pAdvertising->stop();

        NimBLEAdvertisementData advData;
        // NimBLE 2.x API: addData(const uint8_t* data, size_t length)
        advData.addData(_advData, _advDataLen);

        _pAdvertising->setAdvertisementData(advData);

        // 设置设备名称在 Scan Response 中
        NimBLEAdvertisementData scanResponse;
        scanResponse.setName(_deviceName);
        _pAdvertising->setScanResponseData(scanResponse);
    }

// =============================================================================
// nRF52840 mbed - ArduinoBLE
// =============================================================================
#elif defined(SEEED_BLE_MBED_NRF52840)
    // ArduinoBLE 使用 BLEAdvertisingData 类
    BLEAdvertisingData advData;
    
    // 设置 Flags
    advData.setFlags(BLEFlagsBREDRNotSupported | BLEFlagsGeneralDiscoverable);
    
    // 设置 Service Data
    // _buildAdvData() 构建的数据格式:
    // [0-2]: Flags (3 bytes)
    // [3]: Service Data 长度
    // [4]: 0x16 (Service Data type)
    // [5-6]: UUID (0xD2, 0xFC)
    // [7]: BTHome Device Info
    // [8+]: Sensor Data
    //
    // setAdvertisedServiceData(UUID, data, len) 会自动添加 UUID
    // 所以我们需要从 offset 7 开始（跳过 UUID），长度减 2
    uint8_t serviceDataLen = _advData[3] - 1;  // 原始 Service Data 长度（不含类型字节）
    uint8_t btHomeDataLen = serviceDataLen - 2;  // 减去 UUID 的 2 字节
    advData.setAdvertisedServiceData(0xFCD2, &_advData[7], btHomeDataLen);
    
    // 设置设备名称
    advData.setLocalName(_deviceName);
    
    // 应用广播数据
    BLE.setAdvertisingData(advData);

// =============================================================================
// nRF52840 Adafruit - Bluefruit
// =============================================================================
#elif defined(SEEED_BLE_NRF52_BLUEFRUIT)
    Bluefruit.Advertising.clearData();
    Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_FLAGS, &_advData[2], 1);
    Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_SERVICE_DATA, &_advData[5], _advDataLen - 5);
    Bluefruit.Advertising.addName();
#endif
}

void SeeedHADiscoveryBLE::advertise() {
    if (!_running) {
        _log("Warning: BLE not initialized");
        return;
    }

    // 更新包 ID（用于去重）
    _packetId++;

    // 构建广播数据
    updateAdvertiseData();

// =============================================================================
// ESP32 - NimBLE
// =============================================================================
#ifdef SEEED_BLE_ESP32
    if (_pAdvertising) {
        _pAdvertising->start();

        if (_debug) {
            Serial.print("[SeeedBLE] Advertise ID=");
            Serial.print(_packetId);
            Serial.print(", len=");
            Serial.println(_advDataLen);
        }
    }

// =============================================================================
// nRF52840 mbed - ArduinoBLE
// =============================================================================
#elif defined(SEEED_BLE_MBED_NRF52840)
    // 设置广播参数
    BLE.setConnectable(false);  // 不可连接广播
    
    // 开始广播
    BLE.advertise();

    if (_debug) {
        Serial.print("[SeeedBLE] Advertise ID=");
        Serial.print(_packetId);
        Serial.print(", len=");
        Serial.println(_advDataLen);
    }

// =============================================================================
// nRF52840 Adafruit - Bluefruit
// =============================================================================
#elif defined(SEEED_BLE_NRF52_BLUEFRUIT)
    Bluefruit.Advertising.start(0);  // 0 = 无超时

    if (_debug) {
        Serial.print("[SeeedBLE] Advertise ID=");
        Serial.print(_packetId);
        Serial.print(", len=");
        Serial.println(_advDataLen);
    }
#endif
}

void SeeedHADiscoveryBLE::_log(const char* message) {
    if (_debug) {
        Serial.print("[SeeedBLE] ");
        Serial.println(message);
    }
}

void SeeedHADiscoveryBLE::_logf(const char* format, ...) {
    if (_debug) {
        char buffer[128];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        Serial.print("[SeeedBLE] ");
        Serial.println(buffer);
    }
}
