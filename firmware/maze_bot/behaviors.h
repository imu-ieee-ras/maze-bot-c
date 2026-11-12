// behaviors.h - driving behaviors (sürüş davranışları)
#ifndef BEHAVIORS_H
#define BEHAVIORS_H

#include <Arduino.h>

extern float lastLeft;
extern float lastFront;
extern float lastRight;

void avoidObstacles();
void turnRight90();
void turnLeftCorner();
void followStraight(float leftDist);
void keepDistance();
void followLeftWall();

#endif
