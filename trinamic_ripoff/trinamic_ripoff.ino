/*
 Stepper Motor Control with Arduino Motor Control Shield V3.0.
 
 This program drives a bipolar stepper motor. The motor is controlled
 by Arduino pins 10, 11, 12, 13.
 
 The motor should do five revolutions into one and five into another direction.
 
 Using this sketch for longer is not recommended because it will keep the motor under current
 and can cause it to become quite hot.
 
 */
 
#include <Stepper.h>
 
int PWMA  = 3;  // Enable pin 1 on Motor Control Shield  
int PWMB  = 11;  // Enable pin 2 on Motor Control Shield  
int dirA = 12;  // Direction pin dirA on Motor Control Shield
int dirB = 13;  // Direction pin dirB on Motor Control Shield

long stepDelay = 5000;  //minimum seems to be 5000us, otherwise motor doesn't turn well.
const long minStepDelay = 200;
 
const int stepsPerRevolution = 24;  // Change this to fit the number of steps per revolution
                                     // for your motor
int i;
int jj; //used in serialEvent() 
int kk; //used in parseCmd()
int dirToggle;
int stepCount;
int targetStep;
int motorRPM;
int homed;
char inputString[200];         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
boolean terminalEcho = true;
int cmdType;
int cmdArg[5];
#define CMD_LIST_LEN 11
const char *cmdList[CMD_LIST_LEN] = {"DUMMY","MST","ROR","ROL","ECHO","MVP","GAP","GIO","HLD","SAP","SIO"};


// Initialize the stepper library on pins 12 and 13:
Stepper myStepper(stepsPerRevolution, dirA, dirB);            
//Stepper myStepper(stepsPerRevolution, PWMA, dirA, PWMB, dirB);            

 void motorStep(int nstep);
 void motorStepSoft(int nstep);
 void motor_hold(int hold);
 void motorSetRPM(int rpm);
 int motorGetRPM();
 void setEchoMode(boolean mode);
 int parseCmd(char *cmdString);
 
void setup() {
  
  Serial.begin(115200);
  terminalEcho = false;
 
  // Setup PWM pins
  pinMode(PWMA, OUTPUT);
  digitalWrite (PWMA, LOW);
 
  pinMode(PWMB, OUTPUT);
  digitalWrite (PWMB, LOW);  

  // Setup dir pins
  //pinMode(dirA, OUTPUT);
  //digitalWrite (PWMA, LOW);
 
  //pinMode(dirB, OUTPUT);
  //digitalWrite (PWMB, LOW);  

  motorSetRPM(600); //max is 240
  myStepper.setSpeed(600);

  dirToggle = 0;
  stepCount = 0;
  homed = 0;
  strcpy(inputString,"");
  jj=0;
}
 
void loop() {

 if(targetStep > stepCount) {
    //motorStep function goes in units of 4
    //motorStep(4, 0);  
    //stepCount += 4;
    //digitalWrite (PWMA, HIGH);
    //digitalWrite (PWMB, HIGH);  
    myStepper.step(1);
    //digitalWrite (PWMA, LOW);
    //digitalWrite (PWMB, LOW);  
    stepCount += 1;
  }else if(targetStep < stepCount) {
      //motorStep(-4, 0);  
      //stepCount -= 4;
      //digitalWrite (PWMA, HIGH);
      //digitalWrite (PWMB, HIGH);  
      myStepper.step(-1);
      //digitalWrite (PWMA, LOW);
      //digitalWrite (PWMB, LOW);  
      stepCount -= 1;
  }

  //force step count to wrap at +/-30000
  if(stepCount > 30000) stepCount = -30000;
  if(stepCount < -30000) stepCount = 30000;
  
  if (stringComplete) {
    /*
    Serial.println(inputString);
    Serial.print("Got a string of length");
    Serial.print(strlen(inputString));
    Serial.print("!!\n\r");
    */
    parseCmd(inputString);
    if(!strcmp(cmdList[cmdType],"ROR")) targetStep = 30005;
    if(!strcmp(cmdList[cmdType],"ROL")) targetStep = -30005;
    if(!strcmp(cmdList[cmdType],"MVP")) targetStep = cmdArg[0]; //(cmdArg[0]/4)*4;
    if(!strcmp(cmdList[cmdType],"MST")) targetStep = stepCount;
    if(!strcmp(cmdList[cmdType],"SIO")) {
      if(cmdArg[1] == 0) digitalWrite(cmdArg[0],LOW);
      else if(cmdArg[1] == 1) digitalWrite(cmdArg[0],HIGH);
    }
    if(!strcmp(cmdList[cmdType],"GIO")) {
      if(digitalRead(cmdArg[0]) == HIGH) Serial.print("1\n\r");
      else Serial.print("0\n\r");
    }
    if(!strcmp(cmdList[cmdType],"HLD")) motor_hold(cmdArg[0]);
    if(!strcmp(cmdList[cmdType],"SAP")) {
      if(cmdArg[0] == 1) {
        stepCount = cmdArg[1]; //4*(cmdArg[1]/4);
        targetStep = cmdArg[1]; //4*(cmdArg[1]/4);
      }
      if(cmdArg[0] == 3) {
        motorSetRPM(cmdArg[1]);
        myStepper.setSpeed(cmdArg[1]);
      }
    }
    if(!strcmp(cmdList[cmdType],"GAP")) {
      if(cmdArg[0] == 1) {
        Serial.print(stepCount);
        Serial.print("\n\r");
      }
      if(cmdArg[0] == 3) {
        Serial.print(motorGetRPM());
        Serial.print("\n\r");
      }
    }
    // clear the string:
    strcpy(inputString,"");
    stringComplete = false;
    jj = 0;
    cmdType = 0;

  }
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    if(terminalEcho) Serial.print(inChar);
    // add it to the inputString:
    
    if(jj < 199) {
      inputString[jj] = inChar;
      jj++;
      inputString[jj] = '\0';
    }else {//buffer is full.  trash it
      strcpy(inputString,""); 
      jj=0;
    }
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n' || inChar == '\r') {
      stringComplete = true;
    }
  }
}

//low-power, low-torque version
void motorStepSoft(int nstep) {
  for(i=0; i<abs(nstep); i++) {
    if(dirToggle == 1) {
      digitalWrite(dirA,LOW);
      if(nstep>0) digitalWrite(dirB,HIGH);
      else digitalWrite(dirB,LOW);
      dirToggle = 0;
    }else {
      digitalWrite(dirA,HIGH);
      if(nstep>0) digitalWrite(dirB,LOW);
      else digitalWrite(dirB,HIGH);
      dirToggle = 1;
    }
      digitalWrite(PWMA,HIGH);
      digitalWrite(PWMB,LOW);
      delayMicroseconds(stepDelay);
      digitalWrite(PWMA,LOW);
      digitalWrite(PWMB,HIGH);
      delayMicroseconds(stepDelay);
      
  }
}


void motorStep(int nstep, int hold) {
  //enable the drive
  digitalWrite(PWMA,HIGH);
  digitalWrite(PWMB,HIGH);
  nstep = nstep/4;
  for(i=0; i<abs(nstep); i++) {
      if(nstep>0) {
        digitalWrite(dirA,HIGH);
        digitalWrite(dirB,LOW);
        delayMicroseconds(stepDelay);
        digitalWrite(dirA,HIGH);
        digitalWrite(dirB,HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(dirA,LOW);
        digitalWrite(dirB,HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(dirA,LOW);
        digitalWrite(dirB,LOW);
        delayMicroseconds(stepDelay);
      }else {
        digitalWrite(dirA,HIGH);
        digitalWrite(dirB,HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(dirA,HIGH);
        digitalWrite(dirB,LOW);
        delayMicroseconds(stepDelay);
        digitalWrite(dirA,LOW);
        digitalWrite(dirB,LOW);
        delayMicroseconds(stepDelay);
        digitalWrite(dirA,LOW);
        digitalWrite(dirB,HIGH);
        delayMicroseconds(stepDelay);
      }
  }
  //release the drive if not holding
  if(hold == 0) {
    digitalWrite(PWMA,LOW);
    digitalWrite(PWMB,LOW);
  }
}

void motor_hold(int hold) {
  if(hold == 0) {
    digitalWrite(PWMA,LOW);
    digitalWrite(PWMB,LOW);
  }else {
    digitalWrite(PWMA,HIGH);
    digitalWrite(PWMB,HIGH);
  }
}

void motorSetRPM(int rpm) {
  //2 delays per step in motor step routine
  double stepDelayDbl = 60000000./(double)(stepsPerRevolution*2*rpm);
  stepDelay = (long)stepDelayDbl;
  if(stepDelay < minStepDelay) stepDelay = minStepDelay;
}

int motorGetRPM() {
  return (500000L/stepDelay)*60/stepsPerRevolution;
}

void setEchoMode(boolean mode) {
  terminalEcho = mode;  
}

int parseCmd(char *cmdString) {
  char *token;
  //here is the list of commands:
  //ECHO on/off\n
  //MVP ####\n
  //MST\n
  //ROR\n
  //ROL\n
  //SAP ## ##\n //1 sets position (for reference setting), 3 sets speed
  //GAP ##\n
  //SIO ## ##\n
  //GIO ##\n
  //and this is the order:
  //{"DUMMY","MST","ROR","ROL","ECHO","MVP","GAP","GIO","SAP","SIO"};
  token = strtok(cmdString," \n\r");

  for(kk=0; kk<CMD_LIST_LEN; kk++) {
    if(!strcmp(token,cmdList[kk])) break;
  }
  cmdType = kk;

  if(cmdType < 4) { //no arguments
    
  }else if(cmdType < 9) { //one argument
    token = strtok(NULL," \n\r");
    if(!strcmp(cmdList[cmdType],"ECHO")) {
      if(!strcmp(token,"off")) setEchoMode(false);
      else if(!strcmp(token,"on")) setEchoMode(true);
      else ;//error, do nothing
    }else cmdArg[0] = atoi(token);
  }else if(cmdType < CMD_LIST_LEN) { //two arguments
    token = strtok(NULL," \n\r");
    cmdArg[0] = atoi(token);
    token = strtok(NULL," \n\r");
    cmdArg[1] = atoi(token);
  }else return 1; //junk command
  
  return 0;
}

