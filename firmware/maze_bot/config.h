// config.h - pins and tuning values (pinler ve ayar değerleri)
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

    // ULTRASONIC DISTANCE SENSOR PINS
    // left
    const int LEFT_TRIG = 4;    // CHANGED (new wiring, yeni kablolama)
    const int LEFT_ECHO = 11;   // CHANGED
    // front
    const int FRONT_TRIG = 7;
    const int FRONT_ECHO = 8;
    // right
    const int RIGHT_TRIG = 12;  // CHANGED
    const int RIGHT_ECHO = 13;  // CHANGED (onboard LED flickers, normal / karttaki LED titrer, normal)
    //Motors
    const int LEFT_IN1 = 9;     // CHANGED (IN1 = forward / ileri)
    const int LEFT_IN2 = 10;    // CHANGED (IN2 = reverse / geri)
    const int RIGHT_IN3 = 5;    // CHANGED (IN3 = forward / ileri)
    const int RIGHT_IN4 = 6;    // CHANGED (IN4 = reverse / geri)
    // NEW: MPU6050 gyro on I2C (A4 = SDA, A5 = SCL)
    const uint8_t MPU_ADDR = 0x68;
    // NEW: RESERVED pins - do not use (ayrılmış pinler, kullanma):
    // 0/1 = HC-05 Bluetooth (Serial), 2/3 = encoders (enkoderler), A4/A5 = MPU6050
    //TUNING
    const float NO_ECHO_CM = 400.0;   // NEW: no echo after 3 tries = nothing in range (3 denemede yankı yok = menzilde bir şey yok)
    const long ECHO_TIMEOUT_US = 12000; // max wait for echo, µs
    const int MAX_RETIRES = 2;
    const int RETRY_DELAY_MS = 10; // wait after a failed try
    const int SENSOR_GAP_MS = 30;  // wait between sensors
    const float US_PER_CM = 58.0;
    const int MAX_PWM = 160;     // NEW: motors are 6V-rated, battery ~12V -> never exceed (motorlar 6V'luk, pil ~12V, aşma)
    const int TEST_SPEED = 140;  // CHANGED: 200 -> 140 (MAX_PWM altında kalsın)
    const int FRONT_STOP_CM = 20;
    const int DRIVE_SPEED = 150;
    const int TURN_SPEED = 160;  // CHANGED: 220 -> 130 (12V'ta 220 fazla)
    const int TURN_90_MS = 400;
    const int OPEN_CM = 30;
    const int CORNER_FWD_MS = 300;   // forward time around left turn (sola dönüş öncesi/sonrası ileri süre)
    const int   TARGET_LEFT_CM = 10;    // desired distance to left wall (sol duvara hedef mesafe)
    const float KP             = 5.0;   // P gain, tune tonight (P katsayısı, akşam ayarlanacak)
    const int   MAX_CORR       = 60;    // max steering correction (en fazla düzeltme)
    const float LEFT_SCALE = 0.90;   // CHANGED: 1.30 -> 1.00, hardware changed, re-tune (donanım değişti, yeniden ayarla)
    const float RIGHT_SCALE = 1.00;

    // NEW: gyro turn tuning (jiroskop dönüş ayarı)
    const float TURN_STOP_EARLY_DEG = 37.0;      // cut motors this early, inertia finishes the turn (atalet tamamlar)
    const unsigned long TURN_TIMEOUT_MS = 4000; // safety: give up if turn takes too long (güvenlik)
#endif
