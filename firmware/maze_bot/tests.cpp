#include "tests.h"
#include "config.h"
#include "motors.h"

    void motorTest(){
      
#if 0
      Serial.println("Forward");
#endif
      drive(TEST_SPEED,TEST_SPEED); delay(3000);
      stopMotors(); delay(3000);
      
#if 0
      Serial.println("Backward");
#endif
      drive(-TEST_SPEED,-TEST_SPEED); delay(3000);
      stopMotors(); delay(3000);
      
#if 0
      Serial.println("Spin left");
#endif
      drive(-TEST_SPEED,TEST_SPEED);delay(3000);
      stopMotors(); delay(3000);
      
#if 0
      Serial.println("Spin Right");
#endif
      drive(TEST_SPEED,-TEST_SPEED); delay(3000);
      stopMotors(); delay(3000);
      }
    /////////////////////////////////////////////////////////////////////////////////7

          void forwardBackwardTest() {
#if 0
          Serial.println("Forward");
#endif
          drive(TEST_SPEED, TEST_SPEED);     // both wheels forward (iki teker ileri)
          delay(5000);
          stopMotors();
          delay(2000);                       // pause to observe (gözlem molası)
        
#if 0
          Serial.println("Backward");
#endif
          drive(-TEST_SPEED, -TEST_SPEED);   // both wheels backward (iki teker geri)
          delay(5000);
          stopMotors();
          delay(2000);
}
