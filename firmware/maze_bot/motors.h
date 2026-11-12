// motors.h - motor driver (motor sürücü)
#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>

void setMotor(int in_x, int in_y, int speed);
void drive(int leftSpeed, int rightSpeed);
void stopMotors();
void turnDegrees(float degrees, bool toRight);

#endif
