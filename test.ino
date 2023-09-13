#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "driver/rtc_io.h"
#include <SPIFFS.h>
#include <FS.h>
#include <HTTPClient.h>

const char *ssid = "Adi S";
const char *password = "12121212";

#define FILE_PHOTO "/datas/photo.jpg"

// Server Photo
String serverName = "192.168.182.71"; // REPLACE WITH YOUR Raspberry Pi IP ADDRESS
// String serverName = "example.com";   // OR REPLACE WITH YOUR DOMAIN NAME

String serverPath = "/upload.php"; // The default serverPath should be upload.php

const int serverPort = 80;

WiFiClient client;
HTTPClient http;
StaticJsonDocument<300> doc;

// Initialize Telegram BOT
String BOTtoken = "5552131858:AAF8VJrbHO5-u58roh199dQgmY6YjsdgBbE"; // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
String CHAT_ID = "1678655923";

bool sendPhoto = false;

WiFiClientSecure clientTCP;

UniversalTelegramBot bot(BOTtoken, clientTCP);

#define FLASH_LED_PIN 4
bool flashState = LOW;

// Checks for new messages every 1 second.
int botRequestDelay = 3000;
unsigned long lastTimeBotRan;

const int PIRsensor = 13;
const int led = 12;
int PIRstate = LOW; // we start, assuming no motion detected
int val = 0;

// the time we give the sensor to calibrate (approx. 10-60 secs according to datatsheet)
const int calibrationTime = 5; // 30 secs

// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

const int timerInterval = 30000; // time between each HTTP POST image
unsigned long previousMillis = 0;

void configInitCamera()
{
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
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // init with high specs to pre-allocate larger buffers
    if (psramFound())
    {
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10; // 0-63 lower number means higher quality
        config.fb_count = 2;
    }
    else
    {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12; // 0-63 lower number means higher quality
        config.fb_count = 1;
    }

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        delay(1000);
        ESP.restart();
    }

    // Drop down frame size for higher initial frame rate
    sensor_t *s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_CIF); // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
}

void handleNewMessages(int numNewMessages)
{
    Serial.print("Handle New Messages: ");
    Serial.println(numNewMessages);

    for (int i = 0; i < numNewMessages; i++)
    {
        String chat_id = String(bot.messages[i].chat_id);
        if (chat_id != CHAT_ID)
        {
            bot.sendMessage(chat_id, "Unauthorized user", "");
            continue;
        }

        // Print the received message
        String text = bot.messages[i].text;
        Serial.println(text);

        String from_name = bot.messages[i].from_name;
        if (text == "/start")
        {
            String welcome = "Welcome , " + from_name + "\n";
            welcome += "Use the following commands to interact with the ESP32-CAM \n";
            welcome += "/photo : takes a new photo\n";
            welcome += "/flash : toggles flash LED \n";
            bot.sendMessage(CHAT_ID, welcome, "");
        }
        if (text == "/flash")
        {
            flashState = !flashState;
            digitalWrite(FLASH_LED_PIN, flashState);
            Serial.println("Change flash LED state");
        }
        if (text == "/photo")
        {
            sendPhoto = true;
            Serial.println("New photo request");
        }
    }
}

String sendPhotoTelegram()
{
    const char *myDomain = "api.telegram.org";
    String getAll = "";
    String getBody = "";

    camera_fb_t *fb = NULL;
    fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("Camera capture failed");
        delay(1000);
        ESP.restart();
        return "Camera capture failed";
    }

    Serial.println("Connect to " + String(myDomain));

    if (clientTCP.connect(myDomain, 443))
    {

        Serial.println("Connection successful");

        String head = "--c010blind3ngineer\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + CHAT_ID + "\r\n--c010blind3ngineer\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
        String tail = "\r\n--c010blind3ngineer--\r\n";

        uint16_t imageLen = fb->len;
        uint16_t extraLen = head.length() + tail.length();
        uint16_t totalLen = imageLen + extraLen;

        clientTCP.println("POST /bot" + BOTtoken + "/sendPhoto HTTP/1.1");
        clientTCP.println("Host: " + String(myDomain));
        clientTCP.println("Content-Length: " + String(totalLen));
        clientTCP.println("Content-Type: multipart/form-data; boundary=c010blind3ngineer");
        clientTCP.println();
        clientTCP.print(head);

        uint8_t *fbBuf = fb->buf;
        size_t fbLen = fb->len;
        for (size_t n = 0; n < fbLen; n = n + 1024)
        {
            if (n + 1024 < fbLen)
            {
                clientTCP.write(fbBuf, 1024);
                fbBuf += 1024;
            }
            else if (fbLen % 1024 > 0)
            {
                size_t remainder = fbLen % 1024;
                clientTCP.write(fbBuf, remainder);
            }
        }

        clientTCP.print(tail);
        esp_camera_fb_return(fb);

        int waitTime = 10000; // timeout 10 seconds
        long startTimer = millis();
        boolean state = false;

        while ((startTimer + waitTime) > millis())
        {
            Serial.print(".");
            delay(100);
            while (clientTCP.available())
            {
                char c = clientTCP.read();
                if (state == true)
                    getBody += String(c);
                if (c == '\n')
                {
                    if (getAll.length() == 0)
                        state = true;
                    getAll = "";
                }
                else if (c != '\r')
                    getAll += String(c);
                startTimer = millis();
            }
            if (getBody.length() > 0)
                break;
        }
        bot.sendMessage(CHAT_ID, "!!WARNING!! \n Terdeteksi Gerakan Di Ladang", "");
        clientTCP.stop();
        Serial.println(getBody);
    }
    else
    {
        getBody = "Connected to api.telegram.org failed.";
        Serial.println("Connected to api.telegram.org failed.");
    }
    return getBody;
}

String sendPhotoToWebsite()
{
    String getAll;
    String getBody;

    camera_fb_t *fb = NULL;
    fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("Camera capture failed");
        delay(1000);
        ESP.restart();
    }

    Serial.println("Connecting to server: " + serverName);

    if (client.connect(serverName.c_str(), serverPort))
    {
        Serial.println("Connection successful!");
        String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
        String tail = "\r\n--RandomNerdTutorials--\r\n";

        uint32_t imageLen = fb->len;
        uint32_t extraLen = head.length() + tail.length();
        uint32_t totalLen = imageLen + extraLen;

        client.println("POST " + serverPath + " HTTP/1.1");
        client.println("Host: " + serverName);
        client.println("Content-Length: " + String(totalLen));
        client.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
        client.println();
        client.print(head);

        uint8_t *fbBuf = fb->buf;
        size_t fbLen = fb->len;
        for (size_t n = 0; n < fbLen; n = n + 1024)
        {
            if (n + 1024 < fbLen)
            {
                client.write(fbBuf, 1024);
                fbBuf += 1024;
            }
            else if (fbLen % 1024 > 0)
            {
                size_t remainder = fbLen % 1024;
                client.write(fbBuf, remainder);
            }
        }
        client.print(tail);

        esp_camera_fb_return(fb);

        int timoutTimer = 10000;
        long startTimer = millis();
        boolean state = false;

        while ((startTimer + timoutTimer) > millis())
        {
            Serial.print(".");
            delay(100);
            while (client.available())
            {
                char c = client.read();
                if (c == '\n')
                {
                    if (getAll.length() == 0)
                    {
                        state = true;
                    }
                    getAll = "";
                }
                else if (c != '\r')
                {
                    getAll += String(c);
                }
                if (state == true)
                {
                    getBody += String(c);
                }
                startTimer = millis();
            }
            if (getBody.length() > 0)
            {
                break;
            }
        }
        Serial.println();
        client.stop();
        Serial.println(getBody);
    }
    else
    {
        getBody = "Connection to " + serverName + " failed.";
        Serial.println(getBody);
    }
    return getBody;
}

void initSPIFFS()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        ESP.restart();
    }
    else
    {
        delay(500);
        Serial.println("SPIFFS mounted successfully");
    }
}

// Check if photo capture was successful
bool checkPhoto(fs::FS &fs)
{
    File f_pic = fs.open(FILE_PHOTO);
    unsigned int pic_sz = f_pic.size();
    return (pic_sz > 100);
}

// Capture Photo and Save it to SPIFFS
void capturePhotoSaveSpiffs(void)
{
    camera_fb_t *fb = NULL; // pointer
    bool ok = 0;            // Boolean indicating if the picture has been taken correctly
    do
    {
        // Take a photo with the camera
        Serial.println("Taking a photo...");

        fb = esp_camera_fb_get();
        if (!fb)
        {
            Serial.println("Camera capture failed");
            return;
        }
        // Photo file name
        Serial.printf("Picture file name: %s\n", FILE_PHOTO);
        File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);
        // Insert the data in the photo file
        if (!file)
        {
            Serial.println("Failed to open file in writing mode");
        }
        else
        {
            file.write(fb->buf, fb->len); // payload (image), payload length
            Serial.print("The picture has been saved in ");
            Serial.print(FILE_PHOTO);
            Serial.print(" - Size: ");
            Serial.print(file.size());
            Serial.println(" bytes");
        }
        // Close the file
        file.close();
        esp_camera_fb_return(fb);

        // check if file has been correctly saved in SPIFFS
        ok = checkPhoto(SPIFFS);
    } while (!ok);
}

void setup()
{
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    // Init Serial Monitor
    Serial.begin(115200);

    // Set LED Flash as output
    pinMode(FLASH_LED_PIN, OUTPUT);
    digitalWrite(FLASH_LED_PIN, flashState);

    // Set PIR sensor as input and LED as output
    pinMode(PIRsensor, INPUT);
    pinMode(led, OUTPUT);

    // Give some time for the PIR sensor to warm up
    Serial.println("Waiting for the sensor to warm up on first boot");
    delay(calibrationTime * 1000); // Time converted back to miliseconds

    // Blink LED 3 times to indicate PIR sensor warmed up
    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(led, LOW);
    delay(500);
    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(led, LOW);
    delay(500);
    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(led, LOW);
    Serial.println("PIR sensor is ACTIVE");

    // Config and init the camera
    configInitCamera();
    initSPIFFS();
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    // Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    Serial.print("ESP32-CAM IP Address: ");
    Serial.println(WiFi.localIP());
}

void loop()
{
    if (millis() > lastTimeBotRan + botRequestDelay)
    {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        while (numNewMessages)
        {
            Serial.println("got response");
            handleNewMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        if (sendPhoto)
        {
            Serial.println("Preparing photo");
            digitalWrite(FLASH_LED_PIN, HIGH);
            Serial.println("Flash state set to HIGH");
            delay(500);
            sendPhotoTelegram();
            sendPhotoToWebsite();
            sendPhoto = false;
            digitalWrite(FLASH_LED_PIN, LOW);
            Serial.println("Flash state set to LOW");
        }

        http.begin("http://192.168.182.71/api.php");
        int httpCode = http.GET();

        if (httpCode > 0)
        {
            char json[300];
            String payload = http.getString();
            payload.toCharArray(json, 300);
            StaticJsonDocument<300> doc;
            deserializeJson(doc, json);

            const char *nama_sensor = doc["nama_sensor"];
            int status_sensor = doc["status"];

            Serial.print("Parsing Nama Sensor : ");
            Serial.println(nama_sensor);
            Serial.print("Status Sensor : ");
            Serial.println(status_sensor);

            Serial.println(payload);

            if (status_sensor = 1)
            {
                val = digitalRead(PIRsensor);

                if (val == HIGH)
                {
                    digitalWrite(led, HIGH);
                    if (PIRstate == LOW)
                    {
                        // we have just turned on because movement is detected
                        Serial.println("Motion detected!");
                        delay(500);
                        Serial.println("Sending photo to Telegram");
                        digitalWrite(FLASH_LED_PIN, HIGH);
                        sendPhotoTelegram();
                        capturePhotoSaveSpiffs();
                        sendPhotoToWebsite();
                        PIRstate = HIGH;
                        digitalWrite(FLASH_LED_PIN, LOW);
                    }
                }
                else
                {
                    digitalWrite(led, LOW);
                    if (PIRstate == HIGH)
                    {
                        Serial.println("Motion ended!");
                        PIRstate = LOW;
                    }
                }
            }
            else
            {
                Serial.println("Sensor Tidak Aktif");
            }
        }
        else
        {
            Serial.println("Tidak bisa terkoneksi dengan Server");
        }

        http.end();

        lastTimeBotRan = millis();
    }
}