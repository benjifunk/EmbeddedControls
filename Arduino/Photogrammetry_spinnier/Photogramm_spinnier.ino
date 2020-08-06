
#include <Stepper.h>
#include "multiCameraIrControl.h"

const int STEPIN = 11, DIRPIN = 9, ENAB = 10, PHOTOPIN = 5 ; //enable pin is 9

const int stepsPerRevolution = 800;

int stepSpeed = 10;

Stepper stemper(stepsPerRevolution, STEPIN, DIRPIN); //Stepper Init.

int exposuresPerRev = 50; //different numbers of exposures per revolution which may be selected.  
                          //Higher number = smaller angle change
bool runFlag = 0, prevRunFlag = 0; //is scanner running or was it
int stepCount = 1;  // count exposures starting at 1
unsigned long startWait;  //time we start the wait timer
unsigned long currentTime; // current time
long preWait = 500;  // pre exposure pause in milis.  Allows the specimen to settle before exposure.
long postWait = 500;  // post exposure pause in milis.  Allows time for the exposure to finish before moving.
int waitFlag = 0; // 0=ready to move 1=pre-exposure wait 2=post-exposure wait

char input[10];
Canon Camera(PHOTOPIN);

byte incomingByte;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  stemper.setSpeed(stepSpeed); // sets stepper to spin at 60 rpm
  pinMode(ENAB, OUTPUT);
  digitalWrite(ENAB, HIGH);
  
  Serial.println("Stepper Spinner Connected. Initializing variables...");
  Serial.print("Stepper Speed: ");
  Serial.print(stepSpeed*30);
  Serial.println(" RPM");
   
}

void loop() {

  if (runFlag != prevRunFlag) // if the run state has changed
  {
  if (runFlag)
    Serial.println("Running Scan./nSend x to stop/nSend p to pause");
  else 
    Serial.println("Scan Stopped. Send s to start or m for menu.");
  prevRunFlag = runFlag;
  }

if(Serial.available() > 0) // check to see if a command has been sent from user
    {
      incomingByte = Serial.read();
      switch(incomingByte)
      {
        
        case 'x': case 'X':
          if(runFlag)
          {
            runFlag = 0; //toggle runFlag to stop
            waitFlag = 0;
          }
          else Serial.println("Already Stopped");
          break;
          
        case 's': case 'S':
          if(!runFlag)
          {
            runFlag = 1; //toggle runFlag to start
          }
          else Serial.println("Already Running.");
          break;
          
        case 'm': case 'M':
          if(!runFlag)
          {
            bool inmenu = 1;
            while(inmenu)
            {
              Serial.println("Menu:");
              Serial.println("1. Exposures per rev\n2. Pre wait\n3. Post wait\n4. Exit");
              int menuB;
              while(Serial.available()<=0);
              menuB = Serial.parseInt();
              switch(menuB)
              {
                case 1:
                  Serial.print("Exposures per rev [10 - 1000] <");
                  Serial.print(exposuresPerRev);
                  Serial.print("> :");
                  while(Serial.available()<=0);
                  exposuresPerRev = Serial.parseInt();
                  break;
  
                case 2:
                int i;
                  Serial.print("Pre-exposure wait (millisec) <");
                  Serial.print(preWait);
                  Serial.print("> :");
                  while(Serial.available()<=0);
                  preWait = Serial.parseInt();
                  break;
                  
                case 3:
                  Serial.println("Post-exposure wait (millisec) <");
                  Serial.print(postWait);
                  Serial.print("> :");
                  while(Serial.available()<=0);
                  postWait = (int)Serial.parseInt();
                  break;
                case 4: inmenu = 0;
                default: break;
              }
            }
          }
          else Serial.println("Stop current run (x) or wait for it to complete.");
          break;
          
        default:
          Serial.println("Where, oh where did you go wrong?");
          break;
      }
    }

  if (runFlag) // sequence is running
  {
    if (stepCount > exposuresPerRev) // sequence has finished
    {
      runFlag = false;
      Serial.println("Done, buddy.");
    }

    currentTime = millis();

    if (waitFlag == 0 && runFlag)
    {
     startWait = millis();
     waitFlag = 1;
     Serial.print("Exposure ");
     Serial.println(stepCount);
     advanceStepper();
    }

    else if (waitFlag == 1)
    {
      if (currentTime - startWait >= preWait)
      {
        Camera.shutterNow();
        startWait = millis();
        waitFlag = 2;
        stepCount++;
      }
    }
    else //if (waitFlag == 2)
      if (currentTime - startWait >= postWait)
        waitFlag = 0;
  }
}

void advanceStepper(){
  digitalWrite(ENAB, LOW);  //activate stepper driver
        stemper.step(stepsPerRevolution/exposuresPerRev);  //advance stepper
        digitalWrite(ENAB, HIGH); //deactivate stepper driver

}

