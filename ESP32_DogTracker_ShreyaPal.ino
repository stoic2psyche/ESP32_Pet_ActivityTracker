#include "SimulatedQMI8658.h"
#include <esp_sleep.h>
#include "TimeManager.h"
#include <math.h>
#include <LittleFS.h>
#include "ActivityLogger.h"
#include "esp_task_wdt.h"


DummyQMI8658 dummySensor;
TimeManager timeManager;
ActivityLogger logger;
const char* ssid = "Wokwi-GUEST";  // Or your Wi-Fi SSID
const char* password = "";         // Or your Wi-Fi password

// Persistent data across deep sleep
RTC_DATA_ATTR int wakeCount = 0;
RTC_DATA_ATTR int timeUpdateCount = 0;
RTC_DATA_ATTR int motionWakeCount = 0;
RTC_DATA_ATTR uint16_t activityCounters[4] = {0}; // Rest, Walk, Run, Play
RTC_DATA_ATTR unsigned long lastLogTime = 0;

// Callback for simulated motion
void onMotionDetected() {
  Serial.println("Motion callback triggered!");
  motionWakeCount++;
}

void logActivityData(uint32_t timestamp, uint16_t a0, uint16_t a1, uint16_t a2, uint16_t a3) {
  char filename[32];
  snprintf(filename, sizeof(filename), "/%04d%02d%02d.dat", 2025, 4, 6); // Replace with dynamic date if available

  File file = LittleFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending.");
    return;
  }

  file.write((uint8_t*)&timestamp, sizeof(uint32_t));
  file.write((uint8_t*)&a0, sizeof(uint16_t));
  file.write((uint8_t*)&a1, sizeof(uint16_t));
  file.write((uint8_t*)&a2, sizeof(uint16_t));
  file.write((uint8_t*)&a3, sizeof(uint16_t));

  file.close();
  Serial.println("Logged activity data.");
}

void logActivityData(const String &data) {
  String filename = getDateFilename();
  File logFile = LittleFS.open(filename, FILE_APPEND);
  if (!logFile) {
    Serial.println("Failed to open log file!");
    return;
  }

  logFile.println(data);  // Write your formatted sensor/activity data
  logFile.close();
  Serial.println("Logged to " + filename);
}

void enterDeepSleep() {
  Serial.println("Going to sleep now...");

  // Enable timer wake-up after 2 minutes
  esp_sleep_enable_timer_wakeup(2 * 60 * 1000000ULL);

  // Enable wake-up on motion (GPIO 33 LOW)
  esp_sleep_enable_ext0_wakeup((gpio_num_t)DummyQMI8658::MOTION_PIN, 0);

  Serial.flush();
  esp_deep_sleep_start();
}

String classifyActivity(float ax, float ay, float az) {
  float magnitude = sqrt(ax * ax + ay * ay + az * az);

  if (magnitude < 1.05) {
    return "Resting";
  } else if (magnitude < 1.3) {
    return "Walking";
  } else if (magnitude < 1.8) {
    return "Running";
  } else {
    return "Playing";
  }
}

String getDateFilename() {
  time_t now;
  struct tm timeinfo;

  time(&now);
  localtime_r(&now, &timeinfo);

  char filename[32];
  strftime(filename, sizeof(filename), "/logs/%Y%m%d.txt", &timeinfo);

  return String(filename);
}

void setup() {
  Serial.begin(9600);
  delay(1000); // Let Serial monitor initialize
  esp_task_wdt_deinit();  // Turn off the task watchdog timer


  Serial.println("Activity Classification: Ready");

  if (!LittleFS.begin(false)) {
    Serial.println("Failed to mount LittleFS");
  } else {
    Serial.println("LittleFS mounted successfully");
  }
  Serial.println("Mounting LittleFS...");
  if (!LittleFS.begin(true)) {  // 'true' forces format if mount fails
  Serial.println("Failed to mount LittleFS even after format.");
} else {
  Serial.println("LittleFS mounted successfully.");
}

if (!LittleFS.exists("/logs")) {
  LittleFS.mkdir("/logs");
}
logger.begin();

  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

  dummySensor.begin();
  dummySensor.configWakeOnMotion();
  dummySensor.setWakeupMotionEventCallBack(onMotionDetected);

  timeManager.begin(ssid, password);
  timeManager.printCurrentTime();

  Serial.println("\n========== WAKE UP ==========");
  wakeCount++;

  if (reason == ESP_SLEEP_WAKEUP_TIMER) {
    timeUpdateCount++;
    Serial.println("Wake reason: TIMER (2 min timeout)");
    Serial.println("Performing time update...");
  } else if (reason == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("Wake reason: MOTION (GPIO)");
    dummySensor.update(); // simulate motion detection
  } else {
    Serial.println("Wake reason: FIRST BOOT or UNKNOWN");
  }

  Serial.print("Total wake count: "); Serial.println(wakeCount);
  Serial.print("Time updates: "); Serial.println(timeUpdateCount);
  Serial.print("Motion wake-ups: "); Serial.println(motionWakeCount);

  IMUdata acc[3], gyr[3];
  dummySensor.readFromFifo(acc, 3, gyr, 3);
  Serial.println("Accelerometer:");
  for (int i = 0; i < 3; i++) {
    Serial.printf("  X: %.2f Y: %.2f Z: %.2f\n", acc[i].x, acc[i].y, acc[i].z);
    String activity = classifyActivity(acc[i].x, acc[i].y, acc[i].z);
    Serial.println("  Activity: " + activity);

    // Update counters
    if (activity == "Resting") activityCounters[0]++;
    else if (activity == "Walking") activityCounters[1]++;
    else if (activity == "Running") activityCounters[2]++;
    else if (activity == "Playing") activityCounters[3]++;
  }

  Serial.println("Gyroscope:");
  for (int i = 0; i < 3; i++) {
    Serial.printf("  X: %.2f Y: %.2f Z: %.2f\n", gyr[i].x, gyr[i].y, gyr[i].z);
  }

  // Log every 5 wakeups (approx. 10 minutes)
  if (wakeCount % 5 == 0) {
    uint32_t ts = time(nullptr);
    logActivityData(ts, activityCounters[0], activityCounters[1], activityCounters[2], activityCounters[3]);

    // Reset counters after logging
    for (int i = 0; i < 4; i++) activityCounters[i] = 0;
  }

  delay(3000); // Let logs print before sleeping again
  enterDeepSleep();
}

void loop() {
  // Nothing here — deep sleep handled in setup()
  String data = "Activity: Playing at " + timeManager.getFormattedTime(); // or use millis() as fallback
logActivityData(data);
logger.logActivity(1);
}
