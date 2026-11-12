#include "tests.h"
#include "config.h"
#include "motors.h"

    void motorTest(){
      
      Serial.println("Forward");
      drive(TEST_SPEED,TEST_SPEED); delay(3000);
      stopMotors(); delay(3000);
      
      Serial.println("Backward");
      drive(-TEST_SPEED,-TEST_SPEED); delay(3000);
      stopMotors(); delay(3000);
      
      Serial.println("Spin left");
      drive(-TEST_SPEED,TEST_SPEED);delay(3000);
      stopMotors(); delay(3000);
      
      Serial.println("Spin Right");
      drive(TEST_SPEED,-TEST_SPEED); delay(3000);
      stopMotors(); delay(3000);
      }
    /////////////////////////////////////////////////////////////////////////////////7

          void forwardBackwardTest() {
          Serial.println("Forward");
          drive(TEST_SPEED, TEST_SPEED);     // both wheels forward (iki teker ileri)
          delay(5000);
          stopMotors();
          delay(2000);                       // pause to observe (gözlem molası)
        
          Serial.println("Backward");
          drive(-TEST_SPEED, -TEST_SPEED);   // both wheels backward (iki teker geri)
          delay(5000);
          stopMotors();
          delay(2000);
}
