#ifndef SIMULATED_QMI8658_H
#define SIMULATED_QMI8658_H

extern uint16_t activityCounters[4];

#include <Arduino.h>

typedef struct {
  float x;
  float y;
  float z;
} IMUdata;

class DummyQMI8658 {
  public:
    bool begin() {
      Serial.println("QMI8658 initialized (dummy)");
      lastMotionCheck = millis();
      pinMode(MOTION_PIN, INPUT_PULLUP); // Simulate external wake-up
      return true;
    }

    void readFromFifo(IMUdata* acc, int accCount, IMUdata* gyr, int gyrCount) {
      Serial.println("Accelerometer:");
    for (int i = 0; i < accCount; i++) {
    acc[i].x = random(-1000, 1000) / 100.0;
    acc[i].y = random(-1000, 1000) / 100.0;
    acc[i].z = random(-1000, 1000) / 100.0;
    Serial.printf("  X: %.2f Y: %.2f Z: %.2f\n", acc[i].x, acc[i].y, acc[i].z);

    // Add activity classification here
    float magnitude = sqrt(acc[i].x * acc[i].x + acc[i].y * acc[i].y + acc[i].z * acc[i].z);
    String activity;
    if (magnitude < 1.05) {
      activity = "Resting";
    } else if (magnitude < 1.3) {
      activity = "Walking";
    } else if (magnitude < 1.8) {
      activity = "Running";
    } else {
      activity = "Playing";
    }
    Serial.println("  Activity: " + activity);
    extern uint16_t activityCounters[4]; // Tell compiler it's defined elsewhere
    if (activity == "Resting") activityCounters[0]++;
    else if (activity == "Walking") activityCounters[1]++;
    else if (activity == "Running") activityCounters[2]++;
    else if (activity == "Playing") activityCounters[3]++;
  }
    }

    void configWakeOnMotion() {
      Serial.println("Wake-on-motion configured (dummy)");
    }

    void setWakeupMotionEventCallBack(void (*cb)()) {
      motionCallback = cb;
      Serial.println("Motion callback set (dummy)");
    }

    void update() {
      if (millis() - lastMotionCheck > 5000) {
        lastMotionCheck = millis();
        if (motionCallback != nullptr) {
          Serial.println("Motion detected! (dummy)");
          motionCallback();
        }
      }
    }

    static const int MOTION_PIN = 33; // Simulated GPIO pin for motion

  private:
    unsigned long lastMotionCheck;
    void (*motionCallback)() = nullptr;
};

#endif
