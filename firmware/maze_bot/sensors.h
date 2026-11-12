// sensors.h - ultrasonic distance sensors (ultrasonik mesafe sensörleri)
#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

void  sendTrigPulse(int trigPin);                 // send 10us pulse (10us sinyal yolla)
long  readEchoTime(int trigPin, int echoPin);     // echo time in us (yankı süresi, µs)
float readDistance(int trigPin, int echoPin);     // distance in cm, -1 if failed (cm, olmazsa -1)
float readDistanceOrLast(int trigPin, int echoPin, float &lastValue);
void  printSensors();                             // print L/F/R to Serial (Serial'e yaz)

#endif
