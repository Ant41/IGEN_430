#include <AccelStepper.h>
#include <math.h>

// Define pin connections
const int dirPinPitch = 2;
const int stepPinPitch = 3;
const int dirPinYaw = 4;
const int stepPinYaw = 5;
const int dirPinReload = 6;
const int stepPinReload = 7;


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
int yawSpeed = 50*gear_ratio;
int pitchSpeed = 400;
int reloadSpeed = 100;
float angle_to_turn_ratio = 5; //5 degrees per rotation (averaged for datapoints)
float reloadSteps = 0.75*step_resolution; //75% of a rotation will make the release mechanism function
float carriage_travel_ratio = 100; //in mm, the amount of linear travel per second of the reload platform

//computational or PI variables
int orientation = 1; //value of 1-6 dependent on where the ball is relative to the chassis
int yawSteps; //number of steps for the yaw motor to move
int pitchSteps; //number of steps for the pitch motor to move
float phi; //in radians
float thetha; //in radians
float carriage_platform_length; //how far the reloading platform is pulled back
int i;
String input;

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
  
  pinMode(RL1, OUTPUT);
  pinMode(RL2, OUTPUT);
  digitalWrite(RL1, LOW);
  digitalWrite(RL2, LOW);

  //first 4 just check basic functionality, 5 goes through a sweep and back, 6 will allow the motors to be moved 
  Serial.println("Enter 1 for pitch testing, 2 for yaw testing, 3 for turret testing, 4 for release mechanism, 5 for full integration, 6 for jogging motors");
  while(serial.available() != true){
    //wait for command to be given
  }
  
}
void loop() {  

  //while(Serial communication available){
//      if(serial communciation has info){
//        parseinfo();
//        calculateInfo();
//        runMotors();
//        launch();
//        reload();
//      }
//  }

}

void parseInfo(){
  
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
     yawSteps = round(phi*(1/(2*3.14159))*step_resolution*gear_ratio);
  }
  else if(orientation == 2 || orientation == 3){
     yawSteps = -1*round(phi*(1/(2*3.14159))*step_resolution*gear_ratio);
  }
  else if(orientation == 5){
    yawSteps = 0;
  }
 
  //pitch
  if(orientation == 1 || orientation == 3){
     pitchSteps = round(thetha*(1/angle_to_turn_ratio)*step_resolution); 
     need to make sure that initial angle is 90
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

  //move pitch motor
    stepperPitch.moveTo(pitchSteps);
    stepperPitch.setSpeed(pitchSpeed);
    while (stepperPitch.distanceToGo() != 0){ 
      stepperPitch.runSpeedToPosition(); 
    }

  //turret
  i = 0;
  turn motor on and check sign
  while(i<carriage_platform_length*(1/carriage_travel_ratio)*1000){
    delay(1);
    if(Serial.available()>0){
        input = Serial.readStringUntil("/n");
        //turn off motor
        turn motor off
        break;
      }
    i = i + 1;
  }
  turn motor off

}

void launch(){
  //move reload pin
    stepperReload.moveTo(reloadSteps);
    stepperReload.setSpeed(reloadSpeed);
    while (stepperReload.distanceToGo() != 0){ 
      stepperReload.runSpeedToPosition(); 
    }
}

void reload(){
  i = 0;
  turn motor on and check sign
  while(i<carriage_platform_length*(1/carriage_travel_ratio)*1000){
    delay(1);
    //add interrupt if required
    if(Serial.available()>0){
        input = Serial.readStringUntil("/n");
        //turn off motor
        turn motor off
        break;
      }
    i = i + 1;
  }
  turn motor off
}
