// OLD gyro code, to be rewritten by Enes
// gyro.h - MPU6050 gyro (jiroskop)
#ifndef GYRO_H
#define GYRO_H

#include <Arduino.h>
#include <Wire.h>

extern bool  mpuOk;        // is MPU6050 connected? (MPU bağlı mı?)
extern float gyroZOffset;  // reading at rest, deg/s (dururkenki sapma)

bool  mpuInit();
float readGyroZRaw();
float readGyroZ();
void  calibrateGyro();
void  gyroTest();

#endif
