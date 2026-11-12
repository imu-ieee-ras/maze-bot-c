#include "behaviors.h"
#include "config.h"
#include "sensors.h"
#include "motors.h"

    //Last good reading of each sensor (global variables)
    float lastLeft = -1;
    float lastFront = -1;
    float lastRight = -1;

    ///////////// BEHAVIOUR ///////////////////////////////////////////////////////////
    //main dirving logic
    void avoidObstacles(){

     // READ SENSORS
    float left_distance = readDistanceOrLast(LEFT_TRIG,LEFT_ECHO,lastLeft);      
    delay(SENSOR_GAP_MS);
    float front_distance = readDistanceOrLast(FRONT_TRIG,FRONT_ECHO,lastFront);
    delay(SENSOR_GAP_MS);
    float right_distance = readDistanceOrLast(RIGHT_TRIG,RIGHT_ECHO, lastRight);


     // 1-) If sensors couldnt measure stop motors
     if( left_distance == -1 || front_distance == -1 || right_distance == -1){
     stopMotors();
     return;}

     // If there is an obstacle forward change your angle
     if(front_distance < FRONT_STOP_CM){
        // CHANGED: timed spin -> gyro turn (süreli dönüş yerine jiroskopla dönüş)
        if   (left_distance > right_distance){ turnDegrees(90, false);}   // left (sola)
        else                                 { turnDegrees(90, true);}    // right (sağa)
      return;
      }

        // 3) If path clear go forward 
        drive(DRIVE_SPEED, DRIVE_SPEED);
       
      }

     void turnRight90(){
      turnDegrees(90, true);   // CHANGED: gyro turn instead of delay (delay yerine jiroskop)
      }

      void turnLeftCorner(){
        // a bit forward
        drive(DRIVE_SPEED,DRIVE_SPEED);
        delay(CORNER_FWD_MS);
        stopMotors();
        delay(100);

        //spin left
        turnDegrees(90, false);   // CHANGED: gyro turn instead of delay (delay yerine jiroskop)

         // 3) into new corridor (yeni koridora gir)
        drive(DRIVE_SPEED,DRIVE_SPEED);
        delay(CORNER_FWD_MS);
        stopMotors();
        }

    // Go straight keeping left wall at 10 cm (düz git, sol duvarı 10 cm'de tut)
      void followStraight(float leftDist) {
        float error = leftDist - TARGET_LEFT_CM;     // sol mesafe − 10
        float steer = KP * error;                     // direksiyon = 5 × hata
        steer = constrain(steer, -MAX_CORR, MAX_CORR);   // limit steering (direksiyonu sınırla)
         drive(DRIVE_SPEED - steer, DRIVE_SPEED + steer);
          }

    // Keep distance from obstacle in front (öndeki engelle mesafeyi koru)
void keepDistance(){
  float front = readDistanceOrLast(FRONT_TRIG, FRONT_ECHO, lastFront);
  Serial.println(front);

  if (front == -1)       { stopMotors(); }
  else if (front < 12)   {
    drive(-200, -200);   // reverse, scale applied inside drive (geri, scale drive içinde)
    delay(3000);          // reverse duration (geri gitme süresi)
    stopMotors();
  }
  else if (front < 20)   { stopMotors(); }
  else                   { drive(DRIVE_SPEED, DRIVE_SPEED); }
}
    
    void followLeftWall(){
      float left_distance = readDistanceOrLast(LEFT_TRIG,LEFT_ECHO,lastLeft);
      delay(SENSOR_GAP_MS);
      float front_distance = readDistanceOrLast(FRONT_TRIG,FRONT_ECHO,lastFront);
      delay(SENSOR_GAP_MS);
      float right_distance = readDistanceOrLast(RIGHT_TRIG,RIGHT_ECHO,lastRight);

      if(left_distance == -1 || front_distance == -1 || right_distance == -1)
      {stopMotors(); return;}

      // sol el kuralı
      // sol açıksa sola dön
      if(left_distance > OPEN_CM)
      {turnLeftCorner();}
      
      // sol kapalı + ön açık
      else if(front_distance > FRONT_STOP_CM)
      {followStraight(left_distance);
      }
        
      // sol + ön kapalı + sağ açık veya kapalı fark etmez sağa dön (bazen 2 kez peş peşe dönecek loopta)
      
      else
      { turnRight90();}  
      
      }

    //////////////////////////////////////////////////////////////////////////////////
