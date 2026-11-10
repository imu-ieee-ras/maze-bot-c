  #include <Wire.h>   // NEW: I2C library for MPU6050 (MPU6050 için I2C kütüphanesi)

    // ULTRASONIC DISTANCE SENSOR PINS
    // left
    const int LEFT_TRIG = 4;    // CHANGED (new wiring, yeni kablolama)
    const int LEFT_ECHO = 11;   // CHANGED
    // front
    const int FRONT_TRIG = 7;
    const int FRONT_ECHO = 8;
    // right
    const int RIGHT_TRIG = 12;  // CHANGED
    const int RIGHT_ECHO = 13;  // CHANGED (onboard LED flickers, normal / karttaki LED titrer, normal)
    //Motors
    const int LEFT_IN1 = 9;     // CHANGED (IN1 = forward / ileri)
    const int LEFT_IN2 = 10;    // CHANGED (IN2 = reverse / geri)
    const int RIGHT_IN3 = 5;    // CHANGED (IN3 = forward / ileri)
    const int RIGHT_IN4 = 6;    // CHANGED (IN4 = reverse / geri)
    // NEW: MPU6050 gyro on I2C (A4 = SDA, A5 = SCL)
    const uint8_t MPU_ADDR = 0x68;
    // NEW: RESERVED pins - do not use (ayrılmış pinler, kullanma):
    // 0/1 = HC-05 Bluetooth (Serial), 2/3 = encoders (enkoderler), A4/A5 = MPU6050
    
    //Last good reading of each sensor (global variables)
    float lastLeft = -1;
    float lastFront = -1;
    float lastRight = -1;
    //TUNING
    const float NO_ECHO_CM = 400.0;   // NEW: no echo after 3 tries = nothing in range (3 denemede yankı yok = menzilde bir şey yok)
    const long ECHO_TIMEOUT_US = 12000; // max wait for echo, µs
    const int MAX_RETIRES = 2;
    const int RETRY_DELAY_MS = 10; // wait after a failed try
    const int SENSOR_GAP_MS = 30;  // wait between sensors
    const float US_PER_CM = 58.0;
    const int MAX_PWM = 160;     // NEW: motors are 6V-rated, battery ~12V -> never exceed (motorlar 6V'luk, pil ~12V, aşma)
    const int TEST_SPEED = 140;  // CHANGED: 200 -> 140 (MAX_PWM altında kalsın)
    const int FRONT_STOP_CM = 20;
    const int DRIVE_SPEED = 150;
    const int TURN_SPEED = 160;  // CHANGED: 220 -> 130 (12V'ta 220 fazla)
    const int TURN_90_MS = 400;
    const int OPEN_CM = 30;
    const int CORNER_FWD_MS = 300;   // forward time around left turn (sola dönüş öncesi/sonrası ileri süre)
    const int   TARGET_LEFT_CM = 10;    // desired distance to left wall (sol duvara hedef mesafe)
    const float KP             = 5.0;   // P gain, tune tonight (P katsayısı, akşam ayarlanacak)
    const int   MAX_CORR       = 60;    // max steering correction (en fazla düzeltme)
    const float LEFT_SCALE = 0.90;   // CHANGED: 1.30 -> 1.00, hardware changed, re-tune (donanım değişti, yeniden ayarla)
    const float RIGHT_SCALE = 1.00;

    // NEW: gyro turn tuning (jiroskop dönüş ayarı)
    const float TURN_STOP_EARLY_DEG = 37.0;      // cut motors this early, inertia finishes the turn (atalet tamamlar)
    const unsigned long TURN_TIMEOUT_MS = 4000; // safety: give up if turn takes too long (güvenlik)

    // NEW: gyro state (jiroskop durumu)
    bool  mpuOk       = false;   // is MPU6050 connected? (MPU bağlı mı?)
    float gyroZOffset = 0.0;     // reading at rest, deg/s (dururkenki sapma)

    
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


   /////////////////////// NEW: GYRO (MPU6050) ///////////////////////////////////////
   // Gyro gives ROTATION SPEED (deg/s), not angle (jiroskop açı değil DÖNME HIZI verir)
   // angle = sum of (speed x time) -> e.g. 90 deg/s for 1 s = 90 degrees

    // Wake up MPU6050, true if found (MPU'yu uyandır, bulunduysa true)
    bool mpuInit(){
      Wire.begin();
      Wire.beginTransmission(MPU_ADDR);
      Wire.write(0x6B); Wire.write(0x00);          // wake up (uyandır)
      if (Wire.endTransmission() != 0) return false;
      Wire.beginTransmission(MPU_ADDR);
      Wire.write(0x1B); Wire.write(0x00);          // range ±250 deg/s -> 131 units = 1 deg/s
      return (Wire.endTransmission() == 0);
      }

    // Raw rotation speed around vertical axis, deg/s (dikey eksende ham dönme hızı)
    float readGyroZRaw(){
      Wire.beginTransmission(MPU_ADDR);
      Wire.write(0x47);                            // GYRO_ZOUT_H register
      if (Wire.endTransmission(false) != 0) return 0;
      if (Wire.requestFrom(MPU_ADDR, (uint8_t)2) != 2) return 0;
      int16_t raw = (Wire.read() << 8) | Wire.read();
      return raw / 131.0;
      }

    // Rotation speed with bias removed (sapması çıkarılmış dönme hızı)
    float readGyroZ(){
      return readGyroZRaw() - gyroZOffset;
      }

    // Robot must be STILL: average 500 readings = bias (robot DURURKEN 500 ölçüm ortalaması = sapma)
    void calibrateGyro(){
      float sum = 0;
      for (int i = 0; i < 500; i++){ sum += readGyroZRaw(); delay(2); }
      gyroZOffset = sum / 500;
      Serial.print("Gyro offset (deg/s): "); Serial.println(gyroZOffset);
      }

    // Test: rotate robot BY HAND, prints angle (robotu ELLE çevir, açıyı yazar)
    void gyroTest(){
      static float angle = 0;
      static unsigned long lastUs = micros();
      unsigned long now = micros();
      float dt = (now - lastUs) / 1000000.0;       // seconds since last read (son okumadan beri saniye)
      lastUs = now;
      angle += readGyroZ() * dt;                   // speed x time added up (hız x zaman toplanır)
      Serial.print("Angle: "); Serial.println(angle);
      delay(20);
      }
    ////////////////////////////////////////////////////////////////////


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
    //Arduino'ya sinyal geliyorsa INPUT, Arduino sinyal yolluyorsa OUTPUT
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

    // NEW: restart marker + gyro setup (yeniden başlama işareti + jiroskop kurulumu)
    Serial.println("=== RESET ===");
    mpuOk = mpuInit();
    if (mpuOk){
      Serial.println("MPU6050 OK - keep robot still (robotu kıpırdatma)");
      delay(500);
      calibrateGyro();
      }
    else {
      Serial.println("MPU6050 NOT FOUND - timed turns (MPU yok, süreyle dönülecek)");
      }
    }
  
  void loop() {
   //printSensors();
   //delay(200);
   //gyroTest();          // NEW: rotate robot by hand (robotu elle çevir)
   //motorTest();
    avoidObstacles();
   // followLeftWall();
   //forwardBackwardTest();
   //turnRight90();
   //  keepDistance();
  //delay(5000);
  }
