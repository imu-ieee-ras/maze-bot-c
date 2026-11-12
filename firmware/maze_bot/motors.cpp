#include "motors.h"
#include "config.h"
#include "gyro.h"

   /////////////////////// 2-) MOTORS ///////////////////////////////////////////////

    //setMotor: manage a just one motor 
    void setMotor(int in_x,int in_y, int speed){

      speed=constrain(speed,-MAX_PWM,MAX_PWM);  // keep in range   // CHANGED: -255,255 -> MAX_PWM (6V motorları koru)
      
      if(speed>0){
        analogWrite(in_x,speed); //PWM
        digitalWrite(in_y,LOW);
        }

       else if(speed<0){
        digitalWrite(in_x,LOW);
        analogWrite(in_y,-speed); //analogWrite eksi sayı kabul etmez. -(-150) = 150.
        }
        
       else {
        digitalWrite(in_x,LOW);
        digitalWrite(in_y,LOW);
        }
      }

      // Drive both motors
     void drive(int leftSpeed, int rightSpeed){
      setMotor(LEFT_IN1, LEFT_IN2, leftSpeed * LEFT_SCALE);
      setMotor(RIGHT_IN3, RIGHT_IN4, rightSpeed * RIGHT_SCALE); 
      }

      // Stop both motors
      void stopMotors() {
        drive(0,0);
        }

    // NEW: spin in place until gyro says 'degrees' turned (jiroskop 'degrees' derece diyene kadar dön)
    // We choose direction with motors, so only HOW MUCH matters -> fabs()
    // (yönü motorla biz seçiyoruz, sadece NE KADAR döndüğü önemli -> mutlak değer)
    void turnDegrees(float degrees, bool toRight){
      stopMotors(); delay(150);

      if (!mpuOk){                                  // no MPU -> old timed turn (MPU yoksa eski süreli dönüş)
        if (toRight) drive(TURN_SPEED,-TURN_SPEED); else drive(-TURN_SPEED,TURN_SPEED);
        delay(TURN_90_MS * degrees / 90.0);
        stopMotors();
        return;
        }

      float turned = 0;
      unsigned long start  = millis();
      unsigned long lastUs = micros();
      if (toRight) drive(TURN_SPEED,-TURN_SPEED); else drive(-TURN_SPEED,TURN_SPEED);

      while (fabs(turned) < degrees - TURN_STOP_EARLY_DEG){
        unsigned long now = micros();
        float dt = (now - lastUs) / 1000000.0;
        lastUs = now;
        turned += readGyroZ() * dt;                 // add speed x time -> angle (hız x zaman -> açı)
        if (millis() - start > TURN_TIMEOUT_MS) break;   // safety (güvenlik)
        }
      stopMotors();
      delay(150);                                   // let chassis settle (şasi otursun)
      Serial.print("Turned (deg): "); Serial.println(fabs(turned));
      }
