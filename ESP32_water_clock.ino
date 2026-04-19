#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "time.h"
#include <HTTPClient.h>

// ===== LCD =====
#define SDA_PIN 23
#define SCL_PIN 22
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== PINS =====
#define BUZZER_PIN 27
#define BUTTON_PIN 26

// ===== WIFI =====
const char* ssid = "wifi name";
const char* password = "password";

// ===== BLYNK =====
String token = "blynk token";

// ===== TIME =====
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

// ===== ALARM =====
int alarmHour = -1, alarmMinute = -1, alarmSecond = -1;
bool alarmSet = false;
bool alarmTriggered = false;

// ===== DEBOUNCE =====
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;
bool lastButtonState = LOW;
bool buttonState = LOW;

// ===== HTTP =====
String httpGET(String url) {
  HTTPClient http;
  http.begin(url);

  int code = http.GET();
  String payload = "";

  if (code > 0) payload = http.getString();
  else Serial.println("HTTP Error");

  http.end();
  return payload;
}

// ===== FETCH ALARM =====
void fetchAlarm() {
  String url = "https://(region).blynk.cloud/external/api/get?token=" + token + "&(blynk pin)";//change blynk pin and region
  String payload = httpGET(url);
  payload.trim();

  int h, m, s;
  if (sscanf(payload.c_str(), "%d:%d:%d", &h, &m, &s) == 3) {
    alarmHour = h;
    alarmMinute = m;
    alarmSecond = s;
    alarmSet = true;
    alarmTriggered = false;
  }
}

// ===== SEND ALARM =====
void sendAlarm(String timeStr) {
  String url = "https://(region).blynk.cloud/external/api/update?token=" + token + "&(pin)=" + timeStr; //change pin and region
  httpGET(url);
}

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT); // or INPUT_PULLUP if wired that way

  // ===== I2C =====
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  lcd.clear();
  lcd.print("WiFi Connected");
  delay(1000);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  fetchAlarm();
}

void loop() {
  struct tm timeinfo;

  // ===== SERIAL INPUT =====
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    int h, m, s;
    if (sscanf(input.c_str(), "%d:%d:%d", &h, &m, &s) == 3) {
      alarmHour = h;
      alarmMinute = m;
      alarmSecond = s;
      alarmSet = true;
      alarmTriggered = false;

      char buffer[10];
      sprintf(buffer, "%02d:%02d:%02d", h, m, s);
      sendAlarm(String(buffer));
    }
  }

  // ===== AUTO FETCH =====
  static unsigned long lastFetch = 0;
  if (millis() - lastFetch > 30000) {
    fetchAlarm();
    lastFetch = millis();
  }

  // ===== TIME =====
  if (!getLocalTime(&timeinfo)) return;

  // ===== SERIAL OUTPUT =====
  Serial.printf("Date: %02d/%02d/%04d  ",
                timeinfo.tm_mday,
                timeinfo.tm_mon + 1,
                timeinfo.tm_year + 1900);

  Serial.printf("Time: %02d:%02d:%02d\n",
                timeinfo.tm_hour,
                timeinfo.tm_min,
                timeinfo.tm_sec);

  // ===== DEBOUNCED BUTTON =====
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH) {
        digitalWrite(BUZZER_PIN, LOW);
        alarmTriggered = false;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Alarm Stopped");
        delay(1000);
      }
    }
  }

  lastButtonState = reading;

  // ===== ALARM TRIGGER =====
  if (alarmSet &&
      timeinfo.tm_hour == alarmHour &&
      timeinfo.tm_min == alarmMinute &&
      timeinfo.tm_sec == alarmSecond &&
      !alarmTriggered) {

    digitalWrite(BUZZER_PIN, HIGH);
    alarmTriggered = true;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("*** ALARM ***");
    lcd.setCursor(0, 1);
    lcd.print("Wake Up!");
  }

  // ===== LCD DISPLAY =====
  if (!alarmTriggered) {
    lcd.setCursor(0, 0);
    lcd.printf("%02d/%02d/%04d",
               timeinfo.tm_mday,
               timeinfo.tm_mon + 1,
               timeinfo.tm_year + 1900);

    lcd.setCursor(0, 1);
    lcd.printf("%02d:%02d:%02d",
               timeinfo.tm_hour,
               timeinfo.tm_min,
               timeinfo.tm_sec);
  }

  delay(1000);
}