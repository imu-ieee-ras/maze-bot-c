#include "sensors.h"
#include "config.h"

  /////////////////////////////  1-) SENSORS    /////////////////////////////////////////////////////////
  //Trig Function: Send electrical signal to sensor from trig pin to trigger ready to measure
  void sendTrigPulse(int trigPin)
    {
    digitalWrite(trigPin,LOW);
    delayMicroseconds(2); // make sure that the pin is clear
    digitalWrite(trigPin,HIGH);
    delayMicroseconds(10); //Sensor needs 10 micro seconds
    digitalWrite(trigPin,LOW);
    }

    //Echo Function: Measure how long sensor signals traveled
   long readEchoTime(int trigPin,int echoPin){
    sendTrigPulse(trigPin);
    long duration = pulseIn(echoPin,HIGH,ECHO_TIMEOUT_US);
    return duration;
    }

    //Convert duration(microsecond) to cm
    float readDistance(int trigPin, int echoPin){
      for(int i=0;i< MAX_RETIRES;i++){
        long total_time = readEchoTime(trigPin,echoPin);
        if(total_time != 0){return total_time / US_PER_CM;}
        delay(RETRY_DELAY_MS); //It fade old echoes and avoid interference(voice forward 20.58 m during 60milisecond)
        }
        return -1; //All three failed
      }

    float readDistanceOrLast(int trigPin,int echoPin, float &lastValue){
        float flag = readDistance(trigPin,echoPin);
        if (flag == -1){
          lastValue = NO_ECHO_CM;   // CHANGED: path is open, forget old wall (yol açık, eski duvarı unut)
          return NO_ECHO_CM;}
      
        lastValue = flag;
        return flag;}
    
    //343 m/s = 34300 cm/s = 34300 / 1.000.000 cm/µs = 0,0343 cm/µs
    // X = V * T / 2 -> x = (0.0343/2) * T -> X = T/58.3  
    // duration = 580 µs → 580 / 58 = 10 cm.


     void printSensors(){
      float left = readDistance(LEFT_TRIG,LEFT_ECHO);
      delay(SENSOR_GAP_MS); // let echoes fade between sensors
      float front = readDistance(FRONT_TRIG,FRONT_ECHO);
      delay(SENSOR_GAP_MS);
      float right = readDistance(RIGHT_TRIG,RIGHT_ECHO);

      Serial.print("L: ");      Serial.print(left);
      Serial.print("F: ");      Serial.print(front);
      Serial.print("R: ");      Serial.println(right);
      }
    ////////////////////////////////////////////////////////////////////
