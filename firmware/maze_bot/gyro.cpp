// OLD gyro code, to be rewritten by Enes
#include "gyro.h"
#include "config.h"

    // NEW: gyro state (jiroskop durumu)
    bool  mpuOk       = false;   // is MPU6050 connected? (MPU bağlı mı?)
    float gyroZOffset = 0.0;     // reading at rest, deg/s (dururkenki sapma)

   /////////////////////// NEW: GYRO (MPU6050) ///////////////////////////////////////
   // Gyro gives ROTATION SPEED (deg/s), not angle (jiroskop açı değil DÖNME HIZI verir)
   // angle = sum of (speed x time) -> e.g. 90 deg/s for 1 s = 90 degrees

    // Wake up MPU6050, true if found (MPU'yu uyandır, bulunduysa true)
    bool mpuInit(){
      Wire.begin();
      Wire.beginTransmission(MPU_ADDR);

      Wire.write(0x6B); Wire.write(0x00);//1st byte which register,second byte 
      // value to write(also 1 byte) wake up (uyandır)
      
      if (Wire.endTransmission() != 0) return false;
      
      Wire.beginTransmission(MPU_ADDR);
      Wire.write(0x1B); Wire.write(0x00);//range ±250 deg/s -> 131 units = 1 deg/s
      return (Wire.endTransmission() == 0);
      }

    // Raw rotation speed around vertical axis, deg/s (dikey eksende ham dönme hızı)
    float readGyroZRaw(){
      Wire.beginTransmission(MPU_ADDR);
      Wire.write(0x47);                            // GYRO_ZOUT_H register
      if (Wire.endTransmission(false) != 0) return 0;
      if (Wire.requestFrom(MPU_ADDR, (uint8_t)2) != 2) return 0;
      int16_t raw = (Wire.read() << 8) | Wire.read();
      return raw / 131.0;
      }

    // Rotation speed with bias removed (sapması çıkarılmış dönme hızı)
    float readGyroZ(){
      return readGyroZRaw() - gyroZOffset;
      }

    // Robot must be STILL: average 500 readings = bias (robot DURURKEN 500 ölçüm ortalaması = sapma)
    void calibrateGyro(){
      float sum = 0;
      for (int i = 0; i < 500; i++){ sum += readGyroZRaw(); delay(2); }
      gyroZOffset = sum / 500;
      Serial.print("Gyro offset (deg/s): "); Serial.println(gyroZOffset);
      }

    // Test: rotate robot BY HAND, prints angle (robotu ELLE çevir, açıyı yazar)
    void gyroTest(){
      static float angle = 0;
      static unsigned long lastUs = micros();
      unsigned long now = micros();
      float dt = (now - lastUs) / 1000000.0;       // seconds since last read (son okumadan beri saniye)
      lastUs = now;
      angle += readGyroZ() * dt;                   // speed x time added up (hız x zaman toplanır)
      Serial.print("Angle: "); Serial.println(angle);
      delay(20);
      }
    ////////////////////////////////////////////////////////////////////
