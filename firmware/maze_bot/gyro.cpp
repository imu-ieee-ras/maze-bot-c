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
      Wire.setWireTimeout(3000,true); //give up if I2C hangs > 3ms
      Wire.beginTransmission(MPU_ADDR);

      Wire.write(0x6B); Wire.write(0x00);//1st byte which register,second byte 
      // value to write(also 1 byte) wake up (uyandır)
      
      if (Wire.endTransmission() != 0) return false;
      
      Wire.beginTransmission(MPU_ADDR);
      Wire.write(0x1B); Wire.write(0x08); //Gyro_Config +-500 deg/s; 1s=65.5 units
      return (Wire.endTransmission() == 0);
      }

    // Raw rotation speed around vertical axis, deg/s (dikey eksende ham dönme hızı)
    float readGyroZRaw(){
      Wire.beginTransmission(MPU_ADDR);
      Wire.write(0x47);                            // GYRO_ZOUT_H register
      if (Wire.endTransmission(false) != 0) return 0;
      if (Wire.requestFrom(MPU_ADDR, (uint8_t)2) != 2) return 0;
      uint8_t high = Wire.read(); //1st byte high
      uint8_t low = Wire.read(); //2nd byte Low
      int16_t raw = (high << 8) | low; //join 00000011+01000100
      return raw / 65.5; //1deg/s = 65.5 units in +-500range
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
#if 0
      Serial.print("Gyro offset (deg/s): "); Serial.println(gyroZOffset);
#endif
      }

    // Test: rotate robot BY HAND, prints angle (robotu ELLE çevir, açıyı yazar)
    void gyroTest(){
      static float angle = 0;
      static unsigned long lastUs = micros();
      unsigned long now = micros();
      float dt = (now - lastUs) / 1000000.0;       // seconds since last read (son okumadan beri saniye)
      lastUs = now;
      angle += readGyroZ() * dt;                   // speed x time added up (hız x zaman toplanır)
#if 0
      Serial.print("Angle: "); Serial.println(angle);
#endif
      delay(20);
      }

      float heading =0.0; //robot direction left:+ , right:-
      unsigned long headingLastUs = 0; //time of last update	
      void resetHeading(){
	  heading = 0.0;              // 🟨 total = 0 (toplam yön = 0)
	  headingLastUs = micros();   // 🟦 last look time = now (en son bakış saati = şimdi)
	}
	// One notebook row: add speed x time to heading (defterin bir satırı)
	void updateHeading(){
  	unsigned long now = micros();                   // 1. what time is it (saat kaç)
  	float dt = (now - headingLastUs) / 1000000.0;   // 2. seconds passed (geçen süre)
  	headingLastUs = now;                            // 3. write the time (saati deftere yaz)
	  heading += readGyroZ() * dt;                    // 4. speed x time to total (toplama ekle)
	}
    ////////////////////////////////////////////////////////////////////
    

























