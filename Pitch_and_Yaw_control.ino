#include <AccelStepper.h>
#include <math.h>

// Define pin connections
const int dirPinPitch = 2;
const int stepPinPitch = 3;
const int dirPinYaw = 4;
const int stepPinYaw = 5;
const int dirPinReload = 6;
const int stepPinReload = 7;
const int REN = 8;
const int LEN = 9;
const int RPWM = 10;
const int LPWM = 11;


//// Define motor interface type
#define motorInterfaceType 1 //1 means a stepper driver
AccelStepper stepperPitch(motorInterfaceType, stepPinPitch, dirPinPitch);; // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
AccelStepper stepperYaw(motorInterfaceType, stepPinYaw, dirPinYaw);; // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 
AccelStepper stepperReload(motorInterfaceType, stepPinReload, dirPinReload);; // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5


//variables

//o===============o
//         <----x (0,0)
//                  y
//                  |
//                  |
//                  v

//pitch angle starts at 0 on this side (left side) <-------| | ------> pitch angle ends at 180 on this side (right side)
//yaw angle is 0 parallel to net, positive if turning towards the net on the left (pointing to left side), negative otherwise)

//turret and ball positions
float x_ball; //Where ball is hit on x-axis relative to (0,0) coordinate
float y_ball; //Where ball is hit on y-axis relative to (0,0) coordinate
float x_turret; //Where turret is on x-axis relative to (0,0) coordinate
float y_turret; //Where turret is on y-axis relative to (0,0) coordinate

//physical parameters
int gear_ratio = 10; 
int step_resolution = 400; //steps per 1 rotation
int yawSpeed = 100;
int pitchSpeed = 400;
int reloadSpeed = 100;
float angle_to_turn_ratio = 5; //5 degrees per rotation (averaged for datapoints)
float reloadSteps = 0.75*step_resolution; //75% of a rotation will make the release mechanism function
float carriage_travel_ratio = 30; //in mm/s, the amount of linear travel per second of the reload platform

//computational or PI variables
int orientation = 1; //value of 1-6 dependent on where the ball is relative to the chassis
int yawSteps; //number of steps for the yaw motor to move
int pitchSteps; //number of steps for the pitch motor to move
float phi; //in degrees
float phi_old = 0; //in degrees
float thetha; //in degrees
float thetha_old = 0;//in degrees
float carriage_platform_length; //how far the reloading platform is pulled back
int i;
String input;
int jog_loop_flag = 0;

void setup()
{  
  Serial.begin(115200);
  Serial.setTimeout(2); //Milliseconds 
  
  stepperPitch.setMaxSpeed(2000);
  stepperPitch.setAcceleration(2000);
  stepperPitch.setCurrentPosition(1); //need this to make the motor start moving. Keep it at value of 1
  stepperYaw.setMaxSpeed(2000);
  stepperYaw.setAcceleration(2000);
  stepperYaw.setCurrentPosition(1); //need this to make the motor start moving. Keep it at value of 1
  stepperReload.setMaxSpeed(2000);
  stepperReload.setAcceleration(2000);
  stepperReload.setCurrentPosition(1); //need this to make the motor start moving. Keep it at value of 1

  pinMode(REN,OUTPUT); //forward
  pinMode(LEN,OUTPUT); //backward
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);

  digitalWrite(REN, LOW);
  digitalWrite(LEN, LOW);
  
//  pinMode(RL1, OUTPUT);
//  pinMode(RL2, OUTPUT);
//  digitalWrite(RL1, LOW);
//  digitalWrite(RL2, LOW);

  Serial.println("initializing");
  
}
void loop() {  

  //first 4 just check basic functionality, 5 goes through a sweep and back, 6 will allow the motors to be moved 
  Serial.println("Enter 1 for pitch testing, 2 for yaw testing, 3 for turret testing, 4 for release mechanism, 5 for full integration, 6 for jogging motors");
  while(Serial.available() != true){
    //wait for command to be given
  }
  input = Serial.parseInt();

  if(input == "1"){//move pitch 40 back and forth
    thetha = 40;
    pitchSteps = round(thetha*(1/angle_to_turn_ratio)*step_resolution); 
    stepperPitch.moveTo(pitchSteps);
    stepperPitch.setSpeed(pitchSpeed);
    while (stepperPitch.distanceToGo() != 0){ 
      stepperPitch.runSpeedToPosition(); 
    }
    Serial.println("here");

    thetha = 0;
    pitchSteps = round(thetha/360*step_resolution); 
    stepperPitch.moveTo(pitchSteps);
    stepperPitch.setSpeed(pitchSpeed);
    while (stepperPitch.distanceToGo() != 0){ 
      stepperPitch.runSpeedToPosition(); 
    }
    Serial.println("here");
  }
  else if(input == "2"){//move yaw motor 30 degrees each direction 
    phi = 30;
    yawSteps = round(phi/360*step_resolution*gear_ratio);
    stepperYaw.moveTo(yawSteps);
    stepperYaw.setSpeed(yawSpeed);
    while (stepperYaw.distanceToGo() != 0){ 
      stepperYaw.runSpeedToPosition(); 
    }
    Serial.println("here");
    
    phi = 0;
    yawSteps = round(phi/360*step_resolution*gear_ratio);
    stepperYaw.moveTo(yawSteps);
    stepperYaw.setSpeed(yawSpeed);
    while (stepperYaw.distanceToGo() != 0){ 
      stepperYaw.runSpeedToPosition(); 
    }
    Serial.println("here");

  }
  else if(input == "3"){//jog platform forward and back 20cm
    carriage_platform_length = 200; 
    i = 0;
    digitalWrite(REN, HIGH);
    analogWrite(RPWM, 180);
    while(i<carriage_platform_length*(1/carriage_travel_ratio)*1000){
      delay(1);
      //add interrupt if required
      if(Serial.available()>0){
          input = Serial.readStringUntil("/n");
          digitalWrite(REN, LOW);
          analogWrite(RPWM, 0);
          break;
        }
      i = i + 1;
    }
    
    digitalWrite(REN, LOW);
    analogWrite(RPWM, 0);

    Serial.println("here");

    //in reverse
    i = 0;
    digitalWrite(LEN, HIGH);
    analogWrite(LPWM, 180);
    while(i<carriage_platform_length*(1/carriage_travel_ratio)*1000){
      delay(1);
      //add interrupt if required
      if(Serial.available()>0){
          input = Serial.readStringUntil("/n");
          digitalWrite(LEN, LOW);
          analogWrite(LPWM, 0);
          break;
        }
      i = i + 1;
    }
    
    digitalWrite(LEN, LOW);
    analogWrite(LPWM, 0);

    Serial.println("here");

  }
  else if(input == "4"){//move pin in and out
    stepperReload.moveTo(reloadSteps);
    stepperReload.setSpeed(reloadSpeed);
    while (stepperReload.distanceToGo() != 0){ 
      stepperReload.runSpeedToPosition(); 
    }
    Serial.println("here");

    stepperReload.moveTo(0);
    stepperReload.setSpeed(reloadSpeed);
    while (stepperReload.distanceToGo() != 0){ 
      stepperReload.runSpeedToPosition(); 
    }
    Serial.println("here");
  }
  else if(input == "5"){
    parseInfo();
    calculateInfo();
    runMotors();
    launch();
    reload();
  }
  else if(input == "6"){
    jog_loop_flag = 0;
    while(jog_loop_flag == 0){
      Serial.println("Enter 1 to jog pitch, 2 to jog yaw, 3 to jog turret, 4 to jog release, 5 to exit jog function");
      while(Serial.available() != true){
       //wait for command to be given
      }
      input = Serial.parseInt();
      if(input == "1"){
        jog_loop_flag = 0;
        while(jog_loop_flag == 0){
          Serial.println("Enter angle to move in degrees referenced to original position (integer only (-90 to 90)): ");
          while(Serial.available() != true){
           //wait for command to be given
          }
          input = Serial.parseInt();
          thetha = input.toInt();
          pitchSteps = round(thetha*(1/angle_to_turn_ratio)*step_resolution); 
          stepperPitch.moveTo(pitchSteps);
          stepperPitch.setSpeed(pitchSpeed);
          while (stepperPitch.distanceToGo() != 0){ 
            stepperPitch.runSpeedToPosition(); 
          }
          Serial.println("Enter 1 to jog again, 2 to exit");
          while(Serial.available() != true){
           //wait for command to be given
          }
          input = Serial.parseInt();
          if(input == "1"){
            jog_loop_flag = 0;
          }
          else{
            jog_loop_flag = 1;
          }
        }
      }
      else if(input == "2"){
        jog_loop_flag = 0;
        while(jog_loop_flag == 0){
          Serial.println("Enter angle to move in degrees referenced to original position (integer only (-90 to 90)): ");
          while(Serial.available() != true){
           //wait for command to be given
          }
          input = Serial.parseInt();
          phi = input.toInt();
          yawSteps = round(phi/360*step_resolution*gear_ratio);
          stepperYaw.moveTo(yawSteps);
          stepperYaw.setSpeed(yawSpeed);
          while (stepperYaw.distanceToGo() != 0){ 
            stepperYaw.runSpeedToPosition(); 
          }
          Serial.println("Enter 1 to jog again, 2 to exit");
          while(Serial.available() != true){
           //wait for command to be given
          }
          input = Serial.parseInt();
          if(input == "1"){
            jog_loop_flag = 0;
          }
          else{
            jog_loop_flag = 1;
          }
        }
      }
      else if(input == "3"){
        jog_loop_flag = 0;
        while(jog_loop_flag == 0){
          Serial.println("Enter distance to travel in mm (integer only (-200 to 200)): ");
          while(Serial.available() != true){
           //wait for command to be given
          }
          input = Serial.parseInt();
          carriage_platform_length = input.toInt();
          i = 0;
          if(carriage_platform_length > 0){
            digitalWrite(REN, HIGH);
            analogWrite(RPWM, 180);
            while(i<carriage_platform_length*(1/carriage_travel_ratio)*1000){
              delay(1);
              //add interrupt if required
              if(Serial.available()>0){
                  input = Serial.readStringUntil("/n");
                  digitalWrite(REN, LOW);
                  analogWrite(RPWM, 0);
                  break;
                }
              i = i + 1;
            }
            digitalWrite(REN, LOW);
            analogWrite(RPWM, 0);
          }
          else if(carriage_platform_length < 0){
              digitalWrite(LEN, HIGH);
              analogWrite(LPWM, 180);
              while(i<carriage_platform_length*(1/carriage_travel_ratio)*1000){
                delay(1);
                //add interrupt if required
                if(Serial.available()>0){
                    input = Serial.readStringUntil("/n");
                    digitalWrite(LEN, LOW);
                    analogWrite(LPWM, 0);
                    break;
                  }
                i = i + 1;
              }
              digitalWrite(LEN, LOW);
              analogWrite(LPWM, 0);
            }
            Serial.println("Enter 1 to jog again, 2 to exit");
            while(Serial.available() != true){
             //wait for command to be given
            }
            input = Serial.parseInt();
            if(input == "1"){
              jog_loop_flag = 0;
            }
            else{
              jog_loop_flag = 1;
            }
          }
        }
        else if(input == "5"){
          jog_loop_flag = 1;
        }
        else{
          jog_loop_flag = 0;
        }
     }
  }
  else{
    Serial.println("Not a valid entry, please try again");
  }

}

void parseInfo(){
  float phi; //in degrees
  float phi_old; //in degrees
  float thetha; //in degrees
  float thetha_old; //in degrees
  float carriage_platform_length; //how far the reloading platform is pulled back
  float x_ball; //Where ball is hit on x-axis relative to (0,0) coordinate
  float y_ball; //Where ball is hit on y-axis relative to (0,0) coordinate
  float x_turret; //Where turret is on x-axis relative to (0,0) coordinate
  float y_turret; //Where turret is on y-axis relative to (0,0) coordinate

  
  
}

void calculateInfo(){ //used to find the required steps for yaw, pitch, and the turret
  //find orientation of chassis
  
  if(y_turret-y_ball > 0 && x_ball-x_turret < 0){ // ball closer to net than chassis and ball on left of chassis 
    //phi is positive
    orientation = 1;
  }
  else if(y_turret-y_ball > 0 && x_ball-x_turret < 0){ // ball closer to net than chassis and ball on right of chassis 
    //phi is negative
    orientation = 2;
  }
  else if(y_turret-y_ball < 0 && x_ball-x_turret > 0){ // ball farther from net than chassis and ball on left of chassis 
    //phi is negative
    orientation = 3;
  }
  else if(y_turret-y_ball < 0 && x_ball-x_turret < 0){ // ball farther from net than chassis and ball on right of chassis 
    //phi is positive
    orientation = 4;
  }
  else if(abs(y_turret-y_ball) < 0.001){ // ball and turret are roughly same distance from net (accounting for rounding error in float value)
    //phi is zero
    orientation = 5;
  }
  else{ //throw error
    orientation = 6; 
    Serial.println("Error: Orientation values do not work");
  }
  
  //yaw calculations
  phi = atan(abs(y_turret-y_ball)/abs(x_ball-x_turret));

  //yaw
  if(orientation == 1 || orientation == 4){
     yawSteps = round(phi/360*step_resolution*gear_ratio);
  }
  else if(orientation == 2 || orientation == 3){
     yawSteps = -1*round(phi/360*step_resolution*gear_ratio);
  }
  else if(orientation == 5){
    yawSteps = 0;
  }
 
  //pitch
  if(orientation == 1 || orientation == 3){
     pitchSteps = round(thetha*(1/angle_to_turn_ratio)*step_resolution); 
     //need to make sure that initial angle is 0
  }
  else if(orientation == 2 || orientation == 4){
     pitchSteps = -1*round(thetha*(1/angle_to_turn_ratio)*step_resolution);
  }
  else if(orientation == 5){
    pitchSteps = 0;
  }

}

void runMotors(){
  //move yaw motor
    stepperYaw.moveTo(yawSteps);
    stepperYaw.setSpeed(yawSpeed);
    while (stepperYaw.distanceToGo() != 0){ 
      stepperYaw.runSpeedToPosition(); 
    }
    Serial.println("Yaw finished");

  //move pitch motor
    stepperPitch.moveTo(pitchSteps);
    stepperPitch.setSpeed(pitchSpeed);
    while (stepperPitch.distanceToGo() != 0){ 
      stepperPitch.runSpeedToPosition(); 
    }
    Serial.println("Pitch finished");

  //turret
    i = 0;
    //might need to change this to LEN and LPWM
    digitalWrite(REN, HIGH);
    analogWrite(RPWM, 180);
    while(i<carriage_platform_length*(1/carriage_travel_ratio)*1000){
      delay(1);
      if(Serial.available()>0){
          input = Serial.readStringUntil("/n");
          digitalWrite(REN, LOW);
          analogWrite(RPWM, 0);
          break;
        }
      i = i + 1;
    }
    digitalWrite(REN, LOW);
    analogWrite(RPWM, 0);
    Serial.println("turret finished");
}

void launch(){
  //move reload pin
    stepperReload.moveTo(reloadSteps);
    stepperReload.setSpeed(reloadSpeed);
    while (stepperReload.distanceToGo() != 0){ 
      stepperReload.runSpeedToPosition(); 
    }
    Serial.println("Release finished");
}

void reload(){
  i = 0;
  //might need to change this to REN and RPWM
  digitalWrite(LEN, HIGH);
  analogWrite(LPWM, 180);
  while(i<carriage_platform_length*(1/carriage_travel_ratio)*1000){
    delay(1);
    //add interrupt if required
    if(Serial.available()>0){
        input = Serial.readStringUntil("/n");
        digitalWrite(LEN, LOW);
        analogWrite(LPWM, 0);
        break;
      }
    i = i + 1;
  }
  digitalWrite(LEN, LOW);
  analogWrite(LPWM, 0);
  Serial.println("Reload finished");
}
