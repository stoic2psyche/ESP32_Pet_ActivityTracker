#ifndef ACTIVITY_LOGGER_H
#define ACTIVITY_LOGGER_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include "TimeManager.h" // Include to get the current time
extern TimeManager timeManager;

class ActivityLogger {
public:
  bool begin() {
    if (!LittleFS.begin()) {
      Serial.println("Failed to mount LittleFS");
      fsStarted = false;
      return false;
    }
    Serial.println("LittleFS mounted successfully.");
    fsStarted = true;
    return true;
  }

  String getLogFilePath() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time for log file path");
      return "/data/unknown.dat";
    }

    char filename[32];
    snprintf(filename, sizeof(filename), "/data/%04d%02d%02d.dat",
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
    return String(filename);
  }

  void logActivity(uint16_t activityCode) {
    if (!fsStarted) {
      Serial.println("Filesystem not initialized");
      return;
    }

    uint32_t timestamp = timeManager.getEpochTime();  // This assumes getEpochTime() is now public

    String path = getLogFilePath();
    File file = LittleFS.open(path, FILE_APPEND);
    if (!file) {
      Serial.println("Failed to open log file for writing: " + path);
      return;
    }

    file.write((const uint8_t*)&timestamp, sizeof(timestamp));
    file.write((const uint8_t*)&activityCode, sizeof(activityCode));
    file.close();

    Serial.printf("Logged: timestamp = %lu, activity = %u\n", timestamp, activityCode);
  }

private:
  bool fsStarted = false;
};

#endif
