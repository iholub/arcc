//#include <SoftwareSerial.h>
#include <Servo.h>

//SoftwareSerial mySerial(12, 13); // RX, TX

String lPwmStr = "";
String rPwmStr = "";

int lPwm;
int rPwm;

String command = ""; // Stores response of the HC-06 Bluetooth device
String cmdStr = "";
boolean parseSuccess = false;

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


const int bufLength = 20;
char inData[bufLength]; // Allocate some space for the string
char inChar=-1; // Where to store the character read
byte index = 0; // Index into array; where to store the character
boolean readSuccess = false;
boolean readBufferOverflow = false;
boolean clearBuf = false;
boolean cmdUpdateMotor = false;
boolean cmdUpdateServoH = false;
boolean cmdUpdateServoV = false;
boolean cmdShowInfo = false;

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
float voltMotor = 0.0;
float voltCtrl = 0.0;
float voltServo = 0.0;
const float R1 = 46720; 
const float R2 = 9980; 
const float VD = (R1 + R2)/R2;

const float ctrl_R1 = 9970; 
const float ctrl_R2 = 4686; 
const float ctrl_VD = (ctrl_R1 + ctrl_R2)/ctrl_R2;

const float servo_R1 = 9950; 
const float servo_R2 = 4712; 
const float servo_VD = (servo_R1 + servo_R2)/servo_R2;

int voltagePinValue;

String infoStr;
unsigned long time1 = 0;
unsigned long time2 = 0;
unsigned long time3 = 0;
unsigned long readVoltagesTime = 0;

static char outstr[15];

void setup() {
  Serial.begin(115200);
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
      readSuccess = false;
      parseSuccess = false;
      cmdUpdateMotor = false;
      cmdUpdateServoH = false;
      cmdUpdateServoV = false;
      cmdShowInfo = false;

    
  readVoltage();
  
  readCommand();
  
  if (!readSuccess) {
    return;
  }
    boolean statOk = false;
    if (!readBufferOverflow) {
      parseCommand();
      if (parseSuccess) {
        statOk = true;
          if (cmdUpdateMotor) {
              updateMotor(dir1PinA, dir2PinA, speedPinA, &lDirOld, lDir, lPwm);
              updateMotor(dir1PinB, dir2PinB, speedPinB, &rDirOld, rDir, rPwm);
          }
          if (cmdUpdateServoH) {
            updateServo(servoH, hServoDir, &hPos);
          }
          if (cmdUpdateServoV) {
            updateServo(servoV, vServoDir, &vPos);
          }
          if (cmdShowInfo) {
            makeInfo();
          }
      }
    }
     
     
     Serial.print("st: ");
     Serial.print(statOk);
     Serial.print(" cmd: ");
     Serial.print(cmdStr);
     if (cmdShowInfo) {
       Serial.print(" info: ");
       Serial.print(infoStr);
     }
     Serial.println("");
     Serial.flush();
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
         readBufferOverflow = false;
         cmdStr = String(inData);
         doClearBuf();
         break;
       }

       inData[index] = inChar; // Store it
       index++; // Increment where to write next
      if (index >= bufLength - 1) {
        inData[index] = '\0'; // Null terminate the string
        readSuccess = true;
        readBufferOverflow = true;
        cmdStr = String(inData);
        doClearBuf();
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
      int strLen = cmdStr.length();
      int pos = 0;
      boolean err = false;
      while (pos <= strLen - 1) {
          actionCommand = cmdStr.charAt(pos);
          int cmdLength = 0;
          if (actionCommand == 'm') {
            cmdLength = 8;
            if (pos + cmdLength <= strLen - 1) {
              cmdUpdateMotor = true;
               parseMotorCommand(pos);
            } else {
              err = true;
              break;
            }
          } else
          if (actionCommand == 'h') {
            cmdLength = 1;
            if (pos + cmdLength <= strLen - 1) {
              cmdUpdateServoH = true;
              parseServoCommandH(pos);
            } else {
              err = true;
              break;
            }
          } else
          if (actionCommand == 'v') {
            cmdLength = 1;
            if (pos + cmdLength <= strLen - 1) {
              cmdUpdateServoV = true;
              parseServoCommandV(pos);
            } else {
              err = true;
              break;
            }
          } else
          if (actionCommand == 'i')  {
            cmdLength = 0;
            cmdShowInfo = true;
          } else {
              err = true;
              break;
          }

        pos = pos + 1 + cmdLength;
      }
      
      parseSuccess = !err;
}

void parseMotorCommand(int pos) {
      lDir = cmdStr.charAt(pos + 1);
      rDir = cmdStr.charAt(pos + 5);
      lPwmStr = cmdStr.substring(pos + 2, pos + 5);
      rPwmStr = cmdStr.substring(pos + 6, pos + 9);
      lPwm = lPwmStr.toInt();
      rPwm = rPwmStr.toInt();
      //Serial.println(lPwm);
      //      Serial.println(rPwm);
}

void parseServoCommandH(int pos) {
      hServoDir = cmdStr.charAt(pos + 1);
}

void parseServoCommandV(int pos) {
      vServoDir = cmdStr.charAt(pos + 1);
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
  if (time2 - time1 > 1000) {
    time1 = time2;
     voltagePinValue = analogRead(voltPinMotor);
     voltMotor = (voltagePinValue * 5.0 * VD) / 1024.0;
    
     voltagePinValue = analogRead(voltPinCtrl);
     voltCtrl = (voltagePinValue * 5.0 * ctrl_VD) / 1024.0;
  
     voltagePinValue = analogRead(voltPinServo);
     voltServo = (voltagePinValue * 5.0 * servo_VD) / 1024.0;
   readVoltagesTime = millis() - time2;
     
  }
}

void makeInfo() {
  infoStr = "";
  if (!cmdShowInfo) {
    return;
  }
     dtostrf(voltMotor,7, 3, outstr);
     infoStr += " v: m: ";
     infoStr += outstr;
     
     dtostrf(voltCtrl,7, 3, outstr);
     infoStr += " c: ";
     infoStr += outstr;
     
     dtostrf(voltServo,7, 3, outstr);
     infoStr += " s: ";
     infoStr += outstr;
    
     infoStr += " t: ";
     infoStr += readVoltagesTime;
     infoStr += " end";
}

