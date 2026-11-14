// OLD gyro code, to be rewritten by Enes
// gyro.h - MPU6050 gyro (jiroskop)
#ifndef GYRO_H
#define GYRO_H

#include <Arduino.h>
#include <Wire.h>

extern bool  mpuOk;        // is MPU6050 connected? (MPU bağlı mı?)
extern float gyroZOffset;  // reading at rest, deg/s (dururkenki sapma)
extern float heading;    // current direction, deg; left +, right - (şu anki yön)
void resetHeading();     // set current direction as 0 (yönü sıfırla)
void updateHeading();    // add speed x time to heading (yönü güncelle)
bool  mpuInit();
float readGyroZRaw();
float readGyroZ();
void  calibrateGyro();
void  gyroTest();

#endif
