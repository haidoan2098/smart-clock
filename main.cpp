#include <WiFi.h>
#include <time.h>

/* ====== WIFI ====== */
const char *ssid = "Doan Lien";
const char *password = "hoibevydo";

/* ====== NTP  ====== */
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600; // GMT+7
const int daylightOffset_sec = 0;

/* ====== UART  ====== */
#define UART_TX 17
#define UART_RX 16
HardwareSerial uart(2);

/* ====== BUTTON ====== */
#define BUTTON_PIN 0 // Nút BOOT trên ESP32

/* ====== GLOBAL ====== */
struct tm timeinfo;
char timeBuffer[10]; // Giảm kích thước buffer vì chuỗi ngắn hơn (8 ký tự + null)

/* ====== PROTOTYPES ====== */
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
  Serial.println("Nhấn nút BOOT (GPIO 0) để đồng bộ thời gian");
  Serial.println("================================");
}

void loop()
{
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    delay(200); // Chống dội nút

    Serial.println("\n[Button] Nút đã được nhấn");
    syncTimeAndSendUART();

    // Chờ nhả nút để không gửi liên tục
    while (digitalRead(BUTTON_PIN) == LOW)
      ;
  }
}

/* ====== SYNC TIME + SEND UART ====== */
void syncTimeAndSendUART()
{
  connectWiFi();

  Serial.println("[NTP] Đang lấy thời gian...");
  // Thử lấy thời gian tối đa 3 lần nếu thất bại
  int retry = 0;
  while (!getLocalTime(&timeinfo) && retry < 3)
  {
    Serial.println("[Retry] Đang thử lại...");
    retry++;
    delay(1000);
  }

  if (retry >= 3)
  {
    Serial.println("[Lỗi] Không lấy được thời gian từ NTP");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    return;
  }

  Serial.println("[NTP] Đồng bộ thời gian thành công");

  // Format chỉ lấy Giờ:Phút:Giây
  formatTimeString();

  Serial.print("[FORMAT] Chuỗi gửi đi = ");
  Serial.println(timeBuffer);

  // Gửi qua UART cho STM32
  uart.print(timeBuffer);
  uart.print("\n"); // Ký tự kết thúc chuỗi

  Serial.println("[UART] Gửi thành công");

  // Ngắt WiFi để tiết kiệm năng lượng
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("[WiFi] Disconnected");
}

/* ==== KẾT NỐI WIFI ====== */
void connectWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
    return;

  Serial.print("[WiFi] Đang kết nối");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    timeout++;
    if (timeout > 20)
    { // Timeout sau 10s
      Serial.println("\n[WiFi] Kết nối thất bại (Timeout)");
      return;
    }
  }

  Serial.println("\n[WiFi] Kết nối thành công");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

/* ====== ĐỊNH DẠNG CHUỖI (CHỈ GIỜ) ====== */
void formatTimeString()
{
  // Chỉ lấy Giờ:Phút:Giây
  // VD: 16:05:30
  // %02d đảm bảo luôn có 2 chữ số (vd: 5 -> 05)
  sprintf(timeBuffer,
          "%02d:%02d:%02d",
          timeinfo.tm_hour,
          timeinfo.tm_min,
          timeinfo.tm_sec);
}