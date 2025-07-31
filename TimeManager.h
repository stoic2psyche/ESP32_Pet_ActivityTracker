#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <WiFi.h>
#include <time.h>

class TimeManager {
public:
  void begin(const char* ssid, const char* password) {
    connectToWiFi(ssid, password);
    syncWithNTP();
  }

  void printCurrentTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time.");
      return;
    }
    Serial.printf("Current time: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  }

  String getDateString() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      return "no-date";
    }
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
    return String(buffer);
  }

  String getTimeString() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      return "no-time";
    }
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
    return String(buffer);
  }

  String getFormattedTime() {
    time_t now = getEpochTime();  // gets epoch time from system
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(buf);
  }

  time_t getEpochTime() {
    return time(nullptr);  // gets current time from system (RTC or NTP-synced)
  }

private:
  void connectToWiFi(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries < 20) {
      delay(500);
      Serial.print(".");
      tries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWi-Fi connected.");
    } else {
      Serial.println("\nWi-Fi not connected.");
    }
  }

  void syncWithNTP() {
    if (WiFi.status() == WL_CONNECTED) {
      configTime(19800, 0, "pool.ntp.org", "time.nist.gov"); // IST offset = 19800 sec
      Serial.print("Waiting for NTP time");
      struct tm timeinfo;
      int retry = 0;
      while (!getLocalTime(&timeinfo) && retry < 20) {
        delay(500);
        Serial.print(".");
        retry++;
      }
      if (getLocalTime(&timeinfo)) {
        Serial.println("Time synchronized with NTP.");
      } else {
        Serial.println("Failed to synchronize time.");
      }
      WiFi.disconnect(true); // Disconnect after sync to save power
    }
  }
};

#endif
