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

    // Active brake: both inputs HIGH -> motor stops fast (aktif fren: iki giriş HIGH -> motor hızla durur)
    void brakeMotors(){
      digitalWrite(LEFT_IN1, HIGH);  digitalWrite(LEFT_IN2, HIGH);
      digitalWrite(RIGHT_IN3, HIGH); digitalWrite(RIGHT_IN4, HIGH);
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

  float target = toRight ? -degrees : degrees;     // right = minus, left = plus (sağ eksi, sol artı)
  resetHeading();                                  // count from 0 (0'dan say)
  unsigned long start = millis();                  // for timeout (zaman aşımı için)

  while (millis() - start < TURN_TIMEOUT_MS){
    updateHeading();                               // update direction (yönü güncelle)
    float error = target - heading;                // degrees left, sign = direction (kalan açı)

    if (fabs(error) < TURN_TOLERANCE_DEG){         // close enough (yeterince yakın)
      brakeMotors();                               // brake (fren)
      if (fabs(readGyroZ()) < 5.0) break;          // really stopped -> done (gerçekten durduysa bitti)
      continue;                                    // still sliding -> keep checking (kayıyorsa izle)
    }

    int speed = TURN_MIN_SPEED + TURN_KP * fabs(error);   // far = fast, near = slow (uzak hızlı, yakın yavaş)
    if (speed > TURN_SPEED) speed = TURN_SPEED;           // limit (sınırla)
    if (error > 0) drive(-speed, speed);           // need more left (sola dön)
    else           drive(speed, -speed);           // need more right or passed target (sağa dön / geri düzelt)
  }
  stopMotors();
  Serial.print("Turned (deg): "); Serial.println(fabs(heading));
  resetHeading();                                  // new corridor = 0 (yeni koridor = 0)
  }



























