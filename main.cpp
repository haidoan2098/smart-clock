#include <WiFi.h>
#include <time.h>

/* ====== WIFI ====== */
const char *ssid = "TRONG MY";
const char *password = "0702507253";

/* ====== NTP  ====== */
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600; // GMT+7
const int daylightOffset_sec = 0;

/* ====== UART  ====== */
#define UART_TX 17
#define UART_RX 16
HardwareSerial uart(2);

/* ====== BUTTON ====== */
#define BUTTON_PIN 0 // hiện tại đang dùng nút boot

/* ====== GLOBAL ====== */
struct tm timeinfo;
char timeBuffer[30];

/* ====== hàm ====== */
void syncTimeAndSendUART();
void connectWiFi();
void formatTimeString();

void setup()
{
  Serial.begin(115200);
  uart.begin(115200, SERIAL_8N1, UART_RX, UART_TX);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.println("================================");
  Serial.println("ESP32 START");
  Serial.println("Nhấn nút để đồng bộ ");
  Serial.println("================================");
}

void loop()
{
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    delay(200); // chống dội nút

    Serial.println("\nNút đã được nhấn");
    syncTimeAndSendUART();

    // chờ nhả nút
    while (digitalRead(BUTTON_PIN) == LOW)
      ;
  }
}

/* ====== SYNC TIME + SEND UART ====== */
void syncTimeAndSendUART()
{
  connectWiFi();

  Serial.println("[NTP] Đang lấy thời gian...");
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("[Lỗi] không láy được thời gian");
    return;
  }

  Serial.println("[NTP] Đồng bộ thời gian thành công");

  formatTimeString();

  Serial.print("[FORMAT] Chuỗi = ");
  Serial.println(timeBuffer);

  uart.print(timeBuffer);
  uart.print("\n");

  Serial.println("[UART] Gửi thành công");

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("[WiFi] Disconnected");
}

/* ==== kết nối wifi ====== */
void connectWiFi()
{
  Serial.print("[WiFi] Đang kết nối");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n[WiFi] Kết nối thành công");
  Serial.println(WiFi.localIP());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

/* ====== Định dạng chuỗi ====== */
void formatTimeString()
{
  // VD: 16:00:00 25-12-2025
  sprintf(timeBuffer,
          "%02d:%02d:%02d %02d-%02d-%04d",
          timeinfo.tm_hour,
          timeinfo.tm_min,
          timeinfo.tm_sec,
          timeinfo.tm_mday,
          timeinfo.tm_mon + 1,
          timeinfo.tm_year + 1900);
}
