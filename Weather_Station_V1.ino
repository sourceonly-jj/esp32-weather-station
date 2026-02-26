//Jasper's weather station ;)

//Load external libraries
#include <WiFi.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ========= OLED =========
#define SCREEN_WIDTH 128 //Displays size
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C   // try 0x3D if blank
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ========= DHT11 =========
#define DHTPIN 4         // Tell us which pin data is coming from
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ========= WiFi / Time =========
const char* WIFI_SSID = "mytest"; //Change this to whatever your wifi name and password is
const char* WIFI_PASS = "poisonivy";

// UK timezone (handles daylight savings automatically)
const char* TIMEZONE = "GMT0BST,M3.5.0/1,M10.5.0/2";  // UK

void syncTime() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(250);
  }

  if (WiFi.status() == WL_CONNECTED) {
    configTzTime(TIMEZONE, "pool.ntp.org", "time.nist.gov", "time.google.com");
  }
}

bool getTimeString(char* out, size_t outSize) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 2000)) return false;
  strftime(out, outSize, "%H:%M:%S", &timeinfo);
  return true;
}

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);   // OLED I2C pins
  dht.begin();
  delay(2000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found");
    while (true) delay(100);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();

  syncTime();
}

void loop() {
  // Read DHT (DHT11 is slow)
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // C

  // Time string
  char timeStr[16] = "--:--:--";
  bool timeOK = getTimeString(timeStr, sizeof(timeStr));

  // Draw screen
  display.clearDisplay();

  // Time on top
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(timeStr);

  // Temp + humidity
  display.setTextSize(1);
  display.setCursor(0, 28);

  if (isnan(t) || isnan(h)) {
    display.println("DHT11 read failed");
    display.println("Check S/+/- wiring");
  } else {
    display.print("Temp: ");
    display.print(t, 0);
    display.print((char)247);
    display.println("C");

    display.print("Hum : ");
    display.print(h, 0);
    display.println("%");
  }

  // WiFi/time status line
  display.setCursor(0, 54);
  if (WiFi.status() == WL_CONNECTED && timeOK) display.print("Time synced");
  else if (WiFi.status() != WL_CONNECTED) display.print("No WiFi (no time sync)");
  else display.print("Time not set");

  display.display();

  // Serial debug
  Serial.printf("Time: %s | T: %.1fC | H: %.1f%%\n", timeStr, t, h);

  delay(1000); // update every second
}