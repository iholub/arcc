//#include <SoftwareSerial.h>
#include <Servo.h>

//SoftwareSerial mySerial(12, 13); // RX, TX

String lPwmStr = "";
String rPwmStr = "";

int lPwm;
int rPwm;

String command = ""; // Stores response of the HC-06 Bluetooth device
String cmdStr = "";

const int ledPin = 13;  // use the built in LED on pin 13 of the Uno
int state = 0;

// left motor 
const int dir1PinA = 2; // N3 motor shield
const int dir2PinA = 4; // N4 motor shield
const int speedPinA = 5; // ENB motor shield, pwm pin

// right motor
const int dir1PinB = 7; // N2 motor shield
const int dir2PinB = 8; // N1 motor shield
const int speedPinB = 6; // ENA motor shield, pwm pin

const int hServoPin = 9; // Servo library disables analogWrite() (PWM) functionality on pins 9 and 10 
const int vServoPin = 10; // Servo library disables analogWrite() (PWM) functionality on pins 9 and 10


const int bufLength = 11;
char inData[bufLength]; // Allocate some space for the string
char inChar=-1; // Where to store the character read
byte index = 0; // Index into array; where to store the character
int cmdCode = 0;
boolean readSuccess = false;
boolean clearBuf = false;

char lDir = 0;
char rDir = 0;
char lDirOld = 0;
char rDirOld = 0;

Servo servoH;          // horizontal servo
Servo servoV;         // vertical servo

const int servoCenter = 90;
const int servoMin = 10;
const int servoMax = 160;

int hPos = servoCenter;
int vPos = servoCenter;

char hServoDir;
char vServoDir;

char actionCommand;

const int voltPinMotor = 0;
const int voltPinCtrl = 6;
const int voltPinServo = 7;
float vout = 0.0;
const float R1 = 46720; 
const float R2 = 9980; 
const float VD = (R1 + R2)/R2;

const float ctrl_R1 = 10000; 
const float ctrl_R2 = 4700; 
const float ctrl_VD = (ctrl_R1 + ctrl_R2)/ctrl_R2;

int voltagePinValue;

unsigned long time1 = 0;
unsigned long time2 = 0;

void setup() {
  Serial.begin(38400);
  //Serial.begin(9600);
  pinMode(voltPinMotor, INPUT);
  pinMode(voltPinCtrl, INPUT);
  pinMode(voltPinServo, INPUT);
  
  pinMode(ledPin, OUTPUT);
  //digitalWrite(ledPin, LOW);
    
  pinMode(dir1PinA,OUTPUT);
  pinMode(dir2PinA,OUTPUT);
  pinMode(speedPinA,OUTPUT);
  pinMode(dir1PinB,OUTPUT);
  pinMode(dir2PinB,OUTPUT);
  pinMode(speedPinB,OUTPUT);

  servoH.attach(hServoPin);  // set horizontal to digital pin 10
  servoV.attach(vServoPin);  // set vertical servo to digital pin 11
}

void loop() {
    
  readVoltage();
  
  readCommand();
  
  if (readSuccess) {
    parseCommand();
    readSuccess = false;
      switch (actionCommand) {
        case 'm':
          updateMotor(dir1PinA, dir2PinA, speedPinA, &lDirOld, lDir, lPwm);
          updateMotor(dir1PinB, dir2PinB, speedPinB, &rDirOld, rDir, rPwm);
          break;
        case 'h':
          updateServo(servoH, hServoDir, &hPos);
          break;
        case 'v':
          updateServo(servoV, vServoDir, &vPos);
          break;
      }
  }
  if (clearBuf) {
    doClearBuf();
  }
    
}

void updateMotor(int dir1Pin, int dir2Pin, int speedPin, char * dirOldValue, char dirValue, int pwmVal) {
  if (*dirOldValue != dirValue) {
    *dirOldValue = dirValue;
    switch (dirValue) {
      case 'f':    
        digitalWrite(dir1Pin, LOW);
        digitalWrite(dir2Pin, HIGH);
        break;
      case 'b':    
        digitalWrite(dir1Pin, HIGH);
        digitalWrite(dir2Pin, LOW);
        break;
   } 
  }
   analogWrite(speedPin, pwmVal);//Sets speed variable via PWM 
}

void readCommand() {
    while (Serial.available() > 0) {
       inChar = Serial.read(); // Read a character
       if (inChar == 'z') {
         inData[index] = '\0'; // Null terminate the string
         readSuccess = true;
         clearBuf = true;
         cmdCode = 5;
         break;
       }

       inData[index] = inChar; // Store it
       index++; // Increment where to write next
      if (index >= bufLength - 1) {
        clearBuf = true;
        break;
      }
    }
}

void doClearBuf() {
        for (int i = 0;i < bufLength - 1; i++) {
            inData[i] = 0;
        }
        index=0;
        clearBuf = false;
}

void parseCommand() {
      cmdStr = String(inData);
      actionCommand = cmdStr.charAt(0);
      switch (actionCommand) {
        case 'm':
          parseMotorCommand();
          break;
        case 'h':
          parseServoCommandH();
          break;
        case 'v':
          parseServoCommandV();
          break;
      }
}

void parseMotorCommand() {
      lDir = cmdStr.charAt(1);
      rDir = cmdStr.charAt(5);
      lPwmStr = cmdStr.substring(2, 5);
      rPwmStr = cmdStr.substring(6, 9);
      lPwm = lPwmStr.toInt();
      rPwm = rPwmStr.toInt();
      //Serial.println(lPwm);
      //      Serial.println(rPwm);
}

void parseServoCommandH() {
      hServoDir = cmdStr.charAt(1);
}

void parseServoCommandV() {
      vServoDir = cmdStr.charAt(1);
}

void updateServo(Servo servo, char dir, int * pos) {
  int inc;
      switch (dir) {
        case '0':
          inc = -10;
          break;
        case '1':
          inc = 10;
          break;
      }
    int newVal = *pos + inc;
    if (newVal >= servoMin && newVal <= servoMax) {
      *pos = *pos + inc;
    }
    servo.write(*pos);
}

void readVoltage() {
  time2 = millis();
  if (time2 - time1 > 2000) {
    time1 = time2;
     voltagePinValue = analogRead(voltPinMotor);
     vout = (voltagePinValue * 5.0 * VD) / 1024.0;
     Serial.print("voltages, motor: ");
     Serial.print(vout);
    
     voltagePinValue = analogRead(voltPinCtrl);
     vout = (voltagePinValue * 5.0 * ctrl_VD) / 1024.0;
     Serial.print(" ctrl: ");
     Serial.print(vout);
  
     voltagePinValue = analogRead(voltPinServo);
     vout = (voltagePinValue * 5.0 * ctrl_VD) / 1024.0;
     Serial.print(" servo: ");
     Serial.println(vout);
   
     
  }
}

