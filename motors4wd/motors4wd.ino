/*
 */
 
const int m1Dir1Pin = 6;
const int m1Dir2Pin = 7;
const int m1PwmPin = 5;

const int m2Dir1Pin = 2;
const int m2Dir2Pin = 4;
const int m2PwmPin = 3;

const int m3Dir1Pin = 12;
const int m3Dir2Pin = 13;
const int m3PwmPin = 11;

const int m4Dir1Pin = 8;
const int m4Dir2Pin = 9;
const int m4PwmPin = 10;

void setup() {   
  Serial.begin(9600);  

  pinMode(m1Dir1Pin,OUTPUT);
  pinMode(m1Dir2Pin,OUTPUT);
  pinMode(m1PwmPin,OUTPUT);

  pinMode(m2Dir1Pin,OUTPUT);
  pinMode(m2Dir2Pin,OUTPUT);
  pinMode(m2PwmPin,OUTPUT);

  pinMode(m3Dir1Pin,OUTPUT);
  pinMode(m3Dir2Pin,OUTPUT);
  pinMode(m3PwmPin,OUTPUT);

  pinMode(m4Dir1Pin,OUTPUT);
  pinMode(m4Dir2Pin,OUTPUT);
  pinMode(m4PwmPin,OUTPUT);

}

void loop() {
  updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 1);
  updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 1);
  updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 1);
  updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 1);
  delay(5000);



  updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 2);
  delay(3000);
  
  updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 2);
  delay(3000);

  updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 2);
  delay(3000);

  updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 2);
  delay(3000);



  updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 0);
  delay(3000);
  
  updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 0);
  delay(3000);

  updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 0);
  delay(3000);

  updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 0);
  delay(3000);


  updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 3);
  updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 3);
  updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 3);
  updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 3);
  delay(5000);

  motorsStop();
  delay(5000);
}

void updateMotor(int dir1Pin, int dir2Pin, int pwmPin, int cmd) {
  switch (cmd) {
    case 0:
      digitalWrite(dir1Pin, LOW);
      digitalWrite(dir2Pin, LOW);
      break;
    case 1:
      digitalWrite(dir1Pin, LOW);
      digitalWrite(dir2Pin, HIGH);
      break;
    case 2:
      digitalWrite(dir1Pin, HIGH);
      digitalWrite(dir2Pin, LOW);
      break;
    case 3:
      digitalWrite(dir1Pin, HIGH);
      digitalWrite(dir2Pin, HIGH);
      break;
  }
  analogWrite(pwmPin, 100);
}

void motorsStop() {
  updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 0);
  updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 0);
  updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 0);
  updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 0);
}
