//#include <SoftwareSerial.h>
#include <Servo.h>
#include <NewPing.h>

#define TRIGGER_PIN  12  // Arduino pin tied to trigger pin on ping sensor.
#define ECHO_PIN     11  // Arduino pin tied to echo pin on ping sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

unsigned int pingSpeed = 50; // How frequently are we going to send out a ping (in milliseconds). 50ms would be 20 times a second.
unsigned long pingTimer;     // Holds the next ping time.
float pingDistance = 999.0;
boolean isObstacleForward = false;

//SoftwareSerial mySerial(12, 13); // RX, TX

String lPwmStr = "";
String rPwmStr = "";

int lPwm = 0;
int lPwmOld = 0;
int rPwm = 0;
int rPwmOld = 0;

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
boolean cmdStopBeforeObstacle = false;
boolean stopBeforeObstacle = false;

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

unsigned long timeStart = 0;
unsigned long timeEnd = 0;
unsigned long timeSpent = 0;

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

  pingTimer = millis(); // Start now.
}

void loop() {
  // Notice how there's no delays in this sketch to allow you to do other processing in-line while doing distance pings.
  if (millis() >= pingTimer) {   // pingSpeed milliseconds since last ping, do another ping.
    pingTimer += pingSpeed;      // Set the next ping time.
    sonar.ping_timer(echoCheck); // Send out the ping, calls "echoCheck" function every 24uS where you can check the ping status.
  }
  
  timeStart = millis();
      readSuccess = false;
      parseSuccess = false;
      cmdUpdateMotor = false;
      cmdUpdateServoH = false;
      cmdUpdateServoV = false;
      cmdShowInfo = false;
      cmdStopBeforeObstacle = false;
    
    if (stopBeforeObstacle) {
       if (isObstacleForward && (lDir == 'f') && (rDir == 'f')) {
         updateMotorSpeed(speedPinA, &lPwmOld, 0);
         updateMotorSpeed(speedPinB, &rPwmOld, 0);
       } else {
         updateMotorSpeed(speedPinA, &lPwmOld, lPwm);
         updateMotorSpeed(speedPinB, &rPwmOld, rPwm);
       }
    }
  
    
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
            if (!(stopBeforeObstacle && isObstacleForward && (lDir == 'f') && (rDir == 'f'))) {
              updateMotor(dir1PinA, dir2PinA, speedPinA, &lDirOld, lDir, &lPwmOld, lPwm);
              updateMotor(dir1PinB, dir2PinB, speedPinB, &rDirOld, rDir, &rPwmOld, rPwm);
            }
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
     
     timeEnd = millis();
     timeSpent = timeEnd - timeStart;
     Serial.print("{status: ");
     Serial.print(statOk);
     Serial.print(", time: ");
     Serial.print(timeSpent);
     Serial.print(", cmd: ");
     Serial.print(cmdStr);
     Serial.print(", ping: ");
     Serial.print(pingDistance);
     if (cmdShowInfo) {
       Serial.print(", info: ");
       Serial.print(infoStr);
     }
     Serial.println("}");
     Serial.flush();
}

void updateMotor(int dir1Pin, int dir2Pin, int speedPin, char * dirOldValue, char dirValue, int * pwmOld, int pwmVal) {
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
  updateMotorSpeed(speedPin, pwmOld, pwmVal);
}

void updateMotorSpeed(int speedPin, int * pwmOld, int pwmVal) {
    if (*pwmOld != pwmVal) {
      *pwmOld = pwmVal;
      analogWrite(speedPin, pwmVal);//Sets speed variable via PWM 
    }
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
          } else
          if (actionCommand == 'o')  {
            cmdLength = 1;
            cmdStopBeforeObstacle = true;
            parseStopBeforeObstacle(pos);
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

void parseStopBeforeObstacle(int pos) {
  stopBeforeObstacle = cmdStr.charAt(pos + 1) == '1';
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
  if (!cmdShowInfo) {
    return;
  }
    infoStr = "{";
     dtostrf(voltMotor,7, 3, outstr);
     infoStr += "motor: ";
     infoStr += outstr;
     
     dtostrf(voltCtrl,7, 3, outstr);
     infoStr += ", ctrl: ";
     infoStr += outstr;
     
     dtostrf(voltServo,7, 3, outstr);
     infoStr += ", servo: ";
     infoStr += outstr;
 
     infoStr += "}";
}

void echoCheck() { // Timer2 interrupt calls this function every 24uS where you can check the ping status.
  // Don't do anything here!
  if (sonar.check_timer()) { // This is how you check to see if the ping was received.
    // Here's where you can add code.
    pingDistance = sonar.ping_result / US_ROUNDTRIP_CM;
    isObstacleForward = pingDistance < 30;
  }
  // Don't do anything here!
}
