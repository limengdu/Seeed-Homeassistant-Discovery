/**
 * ============================================================================
 * Seeed HA Discovery - Camera Streaming Example
 * Seeed HA Discovery - 摄像头推流示例
 * ============================================================================
 *
 * This example demonstrates how to:
 * 本示例展示如何：
 * 1. Initialize the OV2640 camera on XIAO ESP32-S3 Sense
 *    初始化 XIAO ESP32-S3 Sense 上的 OV2640 摄像头
 * 2. Stream camera video to Home Assistant via MJPEG
 *    通过 MJPEG 将摄像头视频流推送到 Home Assistant
 * 3. Register camera entity for auto-discovery
 *    注册摄像头实体以支持自动发现
 *
 * Hardware Requirements:
 * 硬件要求：
 * - XIAO ESP32-S3 Sense with OV2640 camera module
 *   带 OV2640 摄像头模块的 XIAO ESP32-S3 Sense
 *
 * Camera Stream URL:
 * 摄像头流地址：
 * - Still image: http://<device_ip>/camera
 *   静态图片: http://<设备IP>/camera
 * - MJPEG stream: http://<device_ip>/stream
 *   MJPEG 视频流: http://<设备IP>/stream
 *
 * Software Dependencies:
 * 软件依赖：
 * - ArduinoJson (by Benoit Blanchon)
 * - WebSockets (by Markus Sattler)
 * - ESP32 Arduino Core (includes esp_camera)
 *
 * IMPORTANT: Make sure to select "XIAO_ESP32S3" as board and enable PSRAM!
 * 重要：请确保选择 "XIAO_ESP32S3" 作为开发板并启用 PSRAM！
 *
 * Arduino IDE Settings:
 * Arduino IDE 设置：
 * - Board: "XIAO_ESP32S3"
 * - PSRAM: "OPI PSRAM"
 *   开发板: "XIAO_ESP32S3"
 *   PSRAM: "OPI PSRAM"
 *
 * @author limengdu
 * @version 1.2.0
 */

#include <SeeedHADiscovery.h>
#include "esp_camera.h"

// =============================================================================
// Configuration - Please modify according to your environment
// 配置区域 - 请根据你的环境修改
// =============================================================================

// WiFi Configuration | WiFi 配置
const char* WIFI_SSID = "Your_WiFi_SSID";      // Your WiFi SSID | 你的WiFi名称
const char* WIFI_PASSWORD = "Your_WiFi_Password";  // Your WiFi password | 你的WiFi密码

// =============================================================================
// XIAO ESP32-S3 Sense Camera Pin Configuration
// XIAO ESP32-S3 Sense 摄像头引脚配置
// =============================================================================

#define PWDN_GPIO_NUM     -1    // Power down pin (not used) | 电源关闭引脚（未使用）
#define RESET_GPIO_NUM    -1    // Reset pin (not used) | 重置引脚（未使用）
#define XCLK_GPIO_NUM     10    // External clock | 外部时钟
#define SIOD_GPIO_NUM     40    // I2C SDA | I2C 数据线
#define SIOC_GPIO_NUM     39    // I2C SCL | I2C 时钟线

#define Y9_GPIO_NUM       48    // Data pins Y2-Y9 | 数据引脚 Y2-Y9
#define Y8_GPIO_NUM       11
#define Y7_GPIO_NUM       12
#define Y6_GPIO_NUM       14
#define Y5_GPIO_NUM       16
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       17
#define Y2_GPIO_NUM       15

#define VSYNC_GPIO_NUM    38    // Vertical sync | 垂直同步
#define HREF_GPIO_NUM     47    // Horizontal reference | 水平参考
#define PCLK_GPIO_NUM     13    // Pixel clock | 像素时钟

// =============================================================================
// Camera Configuration | 摄像头配置
// =============================================================================

// Frame size options | 帧大小选项:
// FRAMESIZE_QQVGA   (160x120)
// FRAMESIZE_QVGA    (320x240)
// FRAMESIZE_CIF     (400x296)
// FRAMESIZE_VGA     (640x480)   <- Recommended | 推荐
// FRAMESIZE_SVGA    (800x600)
// FRAMESIZE_XGA     (1024x768)
// FRAMESIZE_SXGA    (1280x1024)
// FRAMESIZE_UXGA    (1600x1200)

#define CAMERA_FRAME_SIZE FRAMESIZE_VGA  // Default frame size | 默认帧大小
#define CAMERA_JPEG_QUALITY 12           // JPEG quality (0-63, lower=better) | JPEG质量（0-63，越低越好）

// =============================================================================
// Global Variables | 全局变量
// =============================================================================

SeeedHADiscovery ha;
WebServer* cameraServer = nullptr;  // Separate server for camera stream | 独立的摄像头服务器

// Camera server task handle | 摄像头服务器任务句柄
TaskHandle_t cameraTaskHandle = nullptr;

// Camera initialized flag | 摄像头初始化标志
bool cameraInitialized = false;

// Mutex for camera access | 摄像头访问互斥锁
SemaphoreHandle_t cameraMutex = nullptr;

// =============================================================================
// Camera Functions | 摄像头功能函数
// =============================================================================

/**
 * Initialize camera
 * 初始化摄像头
 */
bool initCamera() {
    Serial.println("Initializing camera...");

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;  // 20MHz XCLK | 20MHz 外部时钟
    config.frame_size = CAMERA_FRAME_SIZE;
    config.pixel_format = PIXFORMAT_JPEG;  // JPEG format for streaming | JPEG 格式用于流传输
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = CAMERA_JPEG_QUALITY;
    config.fb_count = 2;  // Double buffer for smooth streaming | 双缓冲实现流畅传输

    // Check if PSRAM is available | 检查 PSRAM 是否可用
    if (psramFound()) {
        Serial.println("PSRAM found, using high quality settings");
        config.jpeg_quality = 10;
        config.fb_count = 2;
        config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
        Serial.println("WARNING: No PSRAM found! Camera may not work properly.");
        config.frame_size = FRAMESIZE_QVGA;  // Use smaller frame | 使用较小的帧
        config.fb_location = CAMERA_FB_IN_DRAM;
        config.fb_count = 1;
    }

    // Initialize camera | 初始化摄像头
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }

    // Get camera sensor reference | 获取摄像头传感器引用
    sensor_t* s = esp_camera_sensor_get();
    if (s == nullptr) {
        Serial.println("Failed to get camera sensor");
        return false;
    }

    // Flip image (XIAO ESP32-S3 Sense needs both flips)
    // 翻转图像（XIAO ESP32-S3 Sense 需要双向翻转）
    s->set_vflip(s, 1);    // Vertical flip | 垂直翻转
    s->set_hmirror(s, 1);  // Horizontal mirror | 水平镜像

    // Optional: Adjust image settings | 可选：调整图像设置
    // s->set_brightness(s, 0);     // -2 to 2 | 亮度
    // s->set_contrast(s, 0);       // -2 to 2 | 对比度
    // s->set_saturation(s, 0);     // -2 to 2 | 饱和度
    // s->set_special_effect(s, 0); // 0=None, 1=Negative, 2=Grayscale... | 特效

    Serial.println("Camera initialized successfully!");

    return true;
}

/**
 * Handle MJPEG stream request
 * 处理 MJPEG 流请求
 *
 * This creates a continuous MJPEG stream that can be viewed in browsers
 * and consumed by Home Assistant.
 * 这会创建一个连续的 MJPEG 流，可以在浏览器中查看并被 Home Assistant 使用。
 */
void handleStream() {
    if (!cameraInitialized) {
        cameraServer->send(503, "text/plain", "Camera not initialized");
        return;
    }

    // 获取互斥锁 | Acquire mutex
    if (cameraMutex && xSemaphoreTake(cameraMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        cameraServer->send(503, "text/plain", "Camera busy");
        return;
    }

    WiFiClient client = cameraServer->client();
    
    if (!client.connected()) {
        if (cameraMutex) xSemaphoreGive(cameraMutex);
        Serial.println("Client not connected");
        return;
    }

    // Send MJPEG stream header | 发送 MJPEG 流头
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Connection: keep-alive");
    client.println("Cache-Control: no-cache, no-store, must-revalidate");
    client.println();

    Serial.println("Starting MJPEG stream...");

    unsigned long lastFrameTime = 0;
    const unsigned long frameInterval = 200;  // 5 fps for stability | 5fps 保证稳定性
    unsigned long frameCount = 0;

    while (client.connected()) {
        // Control frame rate | 控制帧率
        unsigned long now = millis();
        if (now - lastFrameTime < frameInterval) {
            delay(5);
            yield();
            continue;
        }
        lastFrameTime = now;

        camera_fb_t* fb = esp_camera_fb_get();
        if (!fb) {
            delay(50);
            continue;
        }

        // Send frame | 发送帧
        if (client.connected()) {
            client.println("--frame");
            client.println("Content-Type: image/jpeg");
            client.printf("Content-Length: %u\r\n\r\n", fb->len);
            
            // 直接发送完整帧 | Send complete frame directly
            size_t written = client.write(fb->buf, fb->len);
            client.println();
            
            frameCount++;
            if (frameCount % 50 == 0) {
                Serial.printf("Streamed %lu frames\n", frameCount);
            }
        }

        esp_camera_fb_return(fb);
        yield();  // Allow other tasks | 允许其他任务运行
    }

    // 释放互斥锁 | Release mutex
    if (cameraMutex) xSemaphoreGive(cameraMutex);

    Serial.printf("Stream ended after %lu frames\n", frameCount);
}

/**
 * Simple image capture handler (for WebServer)
 * 简单图片捕获处理器（用于 WebServer）
 */
void handleSimpleCapture() {
    if (!cameraInitialized) {
        cameraServer->send(503, "text/plain", "Camera not initialized");
        return;
    }

    // 获取互斥锁 | Acquire mutex
    if (cameraMutex && xSemaphoreTake(cameraMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        cameraServer->send(503, "text/plain", "Camera busy");
        return;
    }

    // 尝试捕获多次，提高成功率 | Try capturing multiple times for better success rate
    camera_fb_t* fb = nullptr;
    for (int i = 0; i < 3 && !fb; i++) {
        fb = esp_camera_fb_get();
        if (!fb) {
            delay(50);
        }
    }
    
    if (!fb) {
        if (cameraMutex) xSemaphoreGive(cameraMutex);
        Serial.println("Camera capture failed");
        cameraServer->send(500, "text/plain", "Camera capture failed");
        return;
    }

    Serial.printf("Captured: %u bytes\n", fb->len);

    // 使用 WiFiClient 直接发送 | Use WiFiClient to send directly
    WiFiClient client = cameraServer->client();
    
    // 发送 HTTP 响应头 | Send HTTP response headers
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: image/jpeg");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Cache-Control: no-cache, no-store, must-revalidate");
    client.printf("Content-Length: %u\r\n", fb->len);
    client.println("Connection: close");
    client.println();
    
    // 发送图片数据 | Send image data
    client.write(fb->buf, fb->len);
    
    esp_camera_fb_return(fb);
    
    // 释放互斥锁 | Release mutex
    if (cameraMutex) xSemaphoreGive(cameraMutex);
}

/**
 * Camera server task - runs on Core 0
 * 摄像头服务器任务 - 在核心 0 上运行
 */
void cameraServerTask(void* parameter) {
    Serial.println("Camera server task started on Core 0");
    
    while (true) {
        if (cameraServer) {
            cameraServer->handleClient();
        }
        vTaskDelay(pdMS_TO_TICKS(1));  // Small delay to prevent watchdog | 小延迟防止看门狗
    }
}

/**
 * Setup camera HTTP server
 * 设置摄像头 HTTP 服务器
 */
void setupCameraServer() {
    // 创建互斥锁 | Create mutex
    cameraMutex = xSemaphoreCreateMutex();
    
    cameraServer = new WebServer(82);  // Use port 82 for camera | 使用端口 82 作为摄像头服务器

    // Still image endpoint | 静态图片端点
    cameraServer->on("/camera", HTTP_GET, handleSimpleCapture);
    cameraServer->on("/capture", HTTP_GET, handleSimpleCapture);

    // MJPEG stream endpoint | MJPEG 流端点
    cameraServer->on("/stream", HTTP_GET, handleStream);
    cameraServer->on("/mjpeg", HTTP_GET, handleStream);

    // Info endpoint | 信息端点
    cameraServer->on("/", HTTP_GET, []() {
        String html = R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>XIAO ESP32-S3 Camera</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #0f0c29 0%, #302b63 50%, #24243e 100%);
            min-height: 100vh;
            padding: 20px;
            color: #eee;
        }
        .container { max-width: 800px; margin: 0 auto; }
        h1 {
            color: #00d9ff;
            margin-bottom: 20px;
            font-size: 1.8em;
            text-align: center;
        }
        .card {
            background: rgba(255,255,255,0.1);
            border-radius: 16px;
            padding: 20px;
            margin-bottom: 20px;
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255,255,255,0.1);
        }
        .stream-container {
            text-align: center;
            background: #000;
            border-radius: 12px;
            overflow: hidden;
            margin-bottom: 20px;
        }
        .stream-container img {
            max-width: 100%;
            height: auto;
            display: block;
            margin: 0 auto;
        }
        .links {
            display: flex;
            gap: 10px;
            flex-wrap: wrap;
            justify-content: center;
        }
        .links a {
            background: linear-gradient(135deg, #00d9ff, #00a8cc);
            color: #000;
            padding: 12px 24px;
            border-radius: 8px;
            text-decoration: none;
            font-weight: 600;
            transition: transform 0.2s, box-shadow 0.2s;
        }
        .links a:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 20px rgba(0,217,255,0.4);
        }
        .info { color: #888; text-align: center; font-size: 0.9em; margin-top: 20px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>📷 XIAO ESP32-S3 Sense Camera</h1>
        
        <div class="card">
            <div class="stream-container">
                <img src="/stream" alt="Camera Stream" />
            </div>
            
            <div class="links">
                <a href="/stream" target="_blank">🎬 MJPEG Stream</a>
                <a href="/camera" target="_blank">📸 Capture Photo</a>
            </div>
        </div>
        
        <div class="info">
            <p>Seeed Studio | XIAO ESP32-S3 Sense Camera</p>
        </div>
    </div>
</body>
</html>)";
        cameraServer->send(200, "text/html", html);
    });

    cameraServer->begin();
    Serial.println("Camera server started on port 82");
    
    // 在核心 0 上创建摄像头服务器任务 | Create camera server task on Core 0
    // 这样主循环 (ha.handle()) 可以在核心 1 上不受阻塞地运行
    // This allows main loop (ha.handle()) to run unblocked on Core 1
    xTaskCreatePinnedToCore(
        cameraServerTask,    // 任务函数 | Task function
        "CameraServer",      // 任务名称 | Task name
        8192,                // 栈大小 | Stack size
        NULL,                // 参数 | Parameters
        1,                   // 优先级 | Priority
        &cameraTaskHandle,   // 任务句柄 | Task handle
        0                    // 核心 0 | Core 0
    );
    Serial.println("Camera task created on Core 0");
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
    Serial.println("  Seeed HA Discovery - Camera Stream");
    Serial.println("========================================");
    Serial.println();

    // Check PSRAM | 检查 PSRAM
    if (psramFound()) {
        Serial.printf("PSRAM Size: %d bytes\n", ESP.getPsramSize());
    } else {
        Serial.println("ERROR: PSRAM not found!");
        Serial.println("Please enable PSRAM in Arduino IDE:");
        Serial.println("  Tools -> PSRAM -> OPI PSRAM");
    }

    // Initialize camera | 初始化摄像头
    cameraInitialized = initCamera();

    if (!cameraInitialized) {
        Serial.println("ERROR: Camera initialization failed!");
        Serial.println("Please check:");
        Serial.println("  1. Camera module is properly connected");
        Serial.println("  2. PSRAM is enabled");
        Serial.println("  3. You are using XIAO ESP32-S3 Sense");
    }

    // Configure device info | 配置设备信息
    ha.setDeviceInfo(
        "XIAO Camera",           // Device name | 设备名称
        "XIAO ESP32-S3 Sense",   // Device model | 设备型号
        "1.0.0"                  // Firmware version | 固件版本
    );

    ha.enableDebug(true);

    // Connect WiFi | 连接 WiFi
    Serial.println("Connecting to WiFi...");

    if (!ha.begin(WIFI_SSID, WIFI_PASSWORD)) {
        Serial.println("WiFi connection failed!");
        while (1) {
            delay(1000);
        }
    }

    Serial.println("WiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(ha.getLocalIP().toString().c_str());

    // Start camera server | 启动摄像头服务器
    if (cameraInitialized) {
        setupCameraServer();
    }

    // =========================================================================
    // Register camera entity to Home Assistant
    // 向 Home Assistant 注册摄像头实体
    // =========================================================================

    // Add a camera sensor to report camera status
    // 添加摄像头传感器报告摄像头状态
    SeeedHASensor* cameraSensor = ha.addSensor("camera_status", "Camera Status");
    cameraSensor->setIcon("mdi:camera");

    // Set camera status based on initialization
    // 根据初始化结果设置摄像头状态
    cameraSensor->setValue(cameraInitialized ? 1 : 0);

    // =========================================================================
    // Initialization complete | 完成初始化
    // =========================================================================

    Serial.println();
    Serial.println("========================================");
    Serial.println("  Initialization Complete!");
    Serial.println("========================================");
    Serial.println();

    if (cameraInitialized) {
        Serial.println("Camera URLs:");
        Serial.print("  Still Image: http://");
        Serial.print(ha.getLocalIP().toString().c_str());
        Serial.println(":82/camera");
        Serial.print("  MJPEG Stream: http://");
        Serial.print(ha.getLocalIP().toString().c_str());
        Serial.println(":82/stream");
        Serial.print("  Web UI: http://");
        Serial.print(ha.getLocalIP().toString().c_str());
        Serial.println(":82/");
        Serial.println();
    }

    Serial.println("Add device in Home Assistant:");
    Serial.println("  Settings -> Devices & Services -> Add Integration");
    Serial.println("  Search 'Seeed HA Discovery'");
    Serial.print("  Enter IP: ");
    Serial.println(ha.getLocalIP().toString().c_str());
    Serial.println();
}

void loop() {
    // Handle SeeedHADiscovery events | 处理 SeeedHADiscovery 事件
    ha.handle();

    // 注意：摄像头服务器在核心 0 的单独任务中运行
    // Note: Camera server runs in separate task on Core 0

    // Connection status monitoring | 连接状态监控
    static unsigned long lastCheck = 0;
    static bool wasConnected = false;

    if (millis() - lastCheck > 5000) {
        lastCheck = millis();

        bool connected = ha.isHAConnected();
        if (connected != wasConnected) {
            Serial.println(connected ? "HA Connected" : "HA Disconnected");
            wasConnected = connected;
        }
    }
}

