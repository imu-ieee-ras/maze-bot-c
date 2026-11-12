#include "telemetry.h"
#include "config.h"
#include "sensors.h"
#include "motors.h"

void sendTelemetry(float left, float front, float right, const char *action) {
  Serial.print("T="); Serial.print(millis());
  Serial.print(",L="); Serial.print(left, 1);
  Serial.print(",F="); Serial.print(front, 1);
  Serial.print(",R="); Serial.print(right, 1);
  Serial.print(",ACT="); Serial.println(action);
}

void telemetryOnlyTest() {
  static float lastLeft = -1;
  static float lastFront = -1;
  static float lastRight = -1;

  stopMotors();
  float left = readDistanceOrLast(LEFT_TRIG, LEFT_ECHO, lastLeft);
  delay(SENSOR_GAP_MS);
  float front = readDistanceOrLast(FRONT_TRIG, FRONT_ECHO, lastFront);
  delay(SENSOR_GAP_MS);
  float right = readDistanceOrLast(RIGHT_TRIG, RIGHT_ECHO, lastRight);
  sendTelemetry(left, front, right, "STOP");
  delay(200);
}
