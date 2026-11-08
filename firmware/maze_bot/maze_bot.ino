  
  
    // ULTRASONIC DISTANCE SENSOR PINS
    // left
    const int LEFT_TRIG = 11;
    const int LEFT_ECHO = 12;
    // front
    const int FRONT_TRIG = 7;
    const int FRONT_ECHO = 8;
    // right
    const int RIGHT_TRIG = 4;
    const int RIGHT_ECHO = 2;
    //Motors
    const int LEFT_IN1 = 6;
    const int LEFT_IN2 = 5;
    const int RIGHT_IN3 = 9;
    const int RIGHT_IN4 = 10;

    //Last good reading of each sensor (global variables)
    float lastLeft = -1;
    float lastFront = -1;
    float lastRight = -1;

    //TUNING
    const long ECHO_TIMEOUT_US = 25000; // max wait for echo, µs
    const int MAX_RETIRES = 3;
    const int RETRY_DELAY_MS = 60; // wait after a failed try
    const int SENSOR_GAP_MS = 30;  // wait between sensors
    const float US_PER_CM = 58.0;
    const int TEST_SPEED = 200; 
    const int FRONT_STOP_CM = 20;
    const int DRIVE_SPEED = 150;
    const int TURN_SPEED = 220;
    const int TURN_90_MS = 400;
    const int OPEN_CM = 30;
    const int CORNER_FWD_MS = 300;   // forward time around left turn (sola dönüş öncesi/sonrası ileri süre)
    const int   TARGET_LEFT_CM = 10;    // desired distance to left wall (sol duvara hedef mesafe)
    const float KP             = 5.0;   // P gain, tune tonight (P katsayısı, akşam ayarlanacak)
    const int   MAX_CORR       = 60;    // max steering correction (en fazla düzeltme)
    const float LEFT_SCALE = 1.30;
    const float RIGHT_SCALE = 1.00;

    
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
          return lastValue;}
      
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



   /////////////////////// 2-) MOTORS ///////////////////////////////////////////////

    //setMotor: manage a just one motor 
    void setMotor(int in_x,int in_y, int speed){

      speed=constrain(speed,-255,255);  // keep in range
      
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
      stopMotors();
      delay(100);
        if   (left_distance > right_distance){ drive(-TURN_SPEED,TURN_SPEED);} 
        else                                 { drive(TURN_SPEED,-TURN_SPEED);}
      delay(TURN_90_MS);
      stopMotors();
      return;
      }

        // 3) If path clear go forward 
        drive(DRIVE_SPEED, DRIVE_SPEED);
       
      }

     void turnRight90(){
      stopMotors();
      delay(200);    
      drive(TURN_SPEED,-TURN_SPEED);
      delay(TURN_90_MS);
      stopMotors();
      }

      void turnLeftCorner(){
        // a bit forward
        drive(DRIVE_SPEED,DRIVE_SPEED);
        delay(CORNER_FWD_MS);
        stopMotors();
        delay(100);

        //spin left
        drive(-TURN_SPEED,TURN_SPEED);
        delay(TURN_90_MS);
        stopMotors();
        delay(100);

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

    
  void setup() {
    Serial.begin(9600); // 9600bit data transfer speed
    //Arduino’ya sinyal geliyorsa INPUT, Arduino sinyal yolluyorsa OUTPUT
    pinMode(LEFT_TRIG,OUTPUT);
    pinMode(LEFT_ECHO,INPUT);
    pinMode(FRONT_TRIG,OUTPUT);
    pinMode(FRONT_ECHO,INPUT);
    pinMode(RIGHT_TRIG,OUTPUT);
    pinMode(RIGHT_ECHO,INPUT);
    pinMode(LEFT_IN1,OUTPUT);
    pinMode(LEFT_IN2,OUTPUT);
    pinMode(RIGHT_IN3,OUTPUT);
    pinMode(RIGHT_IN4,OUTPUT);
    stopMotors();
    
    }
  
  void loop() {
   //printSensors();
   //delay(200);
   //motorTest();
   // avoidObstacles();
   // followLeftWall();
   //forwardBackwardTest();
   //turnRight90();
     keepDistance();
  delay(50);
  }






















  
