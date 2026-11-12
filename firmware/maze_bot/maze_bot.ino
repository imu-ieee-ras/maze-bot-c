  #include <Wire.h>   // NEW: I2C library for MPU6050 (MPU6050 için I2C kütüphanesi)
  #include "config.h"
  #include "sensors.h"
  #include "gyro.h"
  #include "motors.h"
  #include "tests.h"
  #include "behaviors.h"

  void setup() {
    Serial.begin(9600); // 9600bit data transfer speed
    //Arduino'ya sinyal geliyorsa INPUT, Arduino sinyal yolluyorsa OUTPUT
    pinMode(LEFT_TRIG,OUTPUT);
    pinMode(LEFT_ECHO,INPUT);
    pinMode(FRONT_TRIG,OUTPUT);
    pinMode(FRONT_ECHO,INPUT);
    pinMode(RIGHT_TRIG,OUTPUT);
    pinMode(RIGHT_ECHO,INPUT);
    pinMode(LEFT_IN1,OUTPUT);
    pinMode(LEFT_IN2,OUTPUT);
    pinMode(RIGHT_IN3,OUTPUT);
    pinMode(RIGHT_IN4,OUTPUT);
    stopMotors();

    // NEW: restart marker + gyro setup (yeniden başlama işareti + jiroskop kurulumu)
    Serial.println("=== RESET ===");
    mpuOk = mpuInit();
    if (mpuOk){
      Serial.println("MPU6050 OK - keep robot still (robotu kıpırdatma)");
      delay(500);
      calibrateGyro();
      }
    else {
      Serial.println("MPU6050 NOT FOUND - timed turns (MPU yok, süreyle dönülecek)");
      }
    }
  
  void loop() {
   //printSensors();
   //delay(200);
   //gyroTest();          // NEW: rotate robot by hand (robotu elle çevir)
   //motorTest();
    avoidObstacles();
   // followLeftWall();
   //forwardBackwardTest();
   //turnRight90();
   //  keepDistance();
  //delay(5000);
  }
