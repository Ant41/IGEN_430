#include <AccelStepper.h>
#include <math.h>

// Define pin connections
const int dirPinPitch = 2;
const int stepPinPitch = 3;
const int dirPinYaw = 4;
const int stepPinYaw = 5;
const int dirPinReload = 7;
const int stepPinReload = 6;
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

//physical parameters
int gear_ratio = 10; 
int step_resolution = 400; //steps per 1 rotation
int yawSpeed = 100;
int pitchSpeed = 800;
int reloadSpeed = 100;
float angle_to_turn_ratio = 5; //5 degrees per rotation (averaged for datapoints)
float reloadSteps = 0.70*step_resolution; //75% of a rotation will make the release mechanism function
float carriage_travel_ratio = 55; //in mm/s, the amount of linear travel per second of the reload platform

//computational or PI variables
//yaw, pitch, carriage_platform_length from PI 
float phi; //in degrees
float thetha; //in degrees
float carriage_platform_length; //how far the reloading platform is pulled back
int yawSteps; //number of steps for the yaw motor to move
int pitchSteps; //number of steps for the pitch motor to move
String input;
int jog_loop_flag = 0;
float i_max;
float Time;
float seconds;
float tenth_seconds;

//parsing variables
const int max_length = 50;
String serial_in;
String debug;
char test[max_length];
char var_1[max_length];
char var_2[max_length];
char var_3[max_length];
int i = 0;
int j = 0;
float num1;
float num2;
float num3;
String data;

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(2000); //Milliseconds 
  
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

  digitalWrite(REN, HIGH);
  digitalWrite(LEN, HIGH);

  resetData();

}

void loop() {
  if(Serial.available()>0){
    data = Serial.readStringUntil('\n');
    parseInfo();
    calculateInfo();
    runMotors();
    launch();
    reload();
    resetData();
    data = Serial.readStringUntil('\n');
  }
}

void parseInfo(){
  //thetha, phi, x
  serial_in = Serial.readStringUntil('\n');
  Serial.println(serial_in);
  data = Serial.readStringUntil('\n');
  for(i = 0;i<serial_in.length();i++){
    test[i] = serial_in[i];
  }
    
  for(i = 1;test[i]!='\0';i++){
    j = 0;
    while(test[i]!='Y'){ //pitch
      var_1[j] = test[i];
      j = j + 1;
      i = i + 1;
    }
    num1 = atof(var_1);
    j = 0;
    i = i + 1;
    while(test[i]!='X'){ //yaw
      var_2[j] = test[i];
      j = j + 1;
      i = i + 1;
    }
    num2 = atof(var_2);
    j = 0;
    i = i + 1;
    while(test[i]!='\0'){ //turret length
      var_3[j] = test[i];
      j = j + 1;
      i = i + 1;
    }
  }
  num3 = atof(var_3);

  thetha = num1; 
  phi = num2;
  carriage_platform_length = num3*1000;
//  Serial.println(num1*2);
//  Serial.println(num2*2);
//  Serial.println(num3*2);
  String str1 = String(thetha);
  String str2 = String(phi);
  String str3 = String(carriage_platform_length);
//  Serial.println(str1 + " " + str2 + " " + str3);
}

void calculateInfo(){ //used to find the required steps for yaw and pitch
  //yaw
  yawSteps = round(phi/360*step_resolution*gear_ratio);
 
  //pitch
  pitchSteps = round(thetha*(1/angle_to_turn_ratio)*step_resolution); 
//  Serial.println(pitchSteps);
}

void runMotors(){ //PIN STARTS OUT
//  move yaw motor
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

    //move reload pin
    Serial.println("reload in run motor");
    stepperReload.moveTo(-reloadSteps);
    stepperReload.setSpeed(reloadSpeed);
    while (stepperReload.distanceToGo() != 0){ 
      stepperReload.runSpeedToPosition(); 
    }

  //pull turret back 
    i = 0;
    Time = abs(carriage_platform_length/carriage_travel_ratio);
    seconds = int((Time*100-(int((Time*100))%100))/100); //(1.55*100 - 55)/100
    tenth_seconds = Time-seconds;
    digitalWrite(LEN, HIGH);
    run_drill_back(seconds,tenth_seconds);
}

void launch(){
  //move reload pin
    stepperReload.moveTo(0);
    stepperReload.setSpeed(reloadSpeed);
    while (stepperReload.distanceToGo() != 0){ 
      stepperReload.runSpeedToPosition(); 
    }
    Serial.println("reload 1");
}

void reload(){ //pull the platform back to the front of the turret
  i = 0;
  digitalWrite(REN, HIGH);
  run_drill_forward(seconds,tenth_seconds);

  Serial.println("reload 2");
  stepperReload.moveTo(-reloadSteps);
  stepperReload.setSpeed(reloadSpeed);
  while (stepperReload.distanceToGo() != 0){ 
    stepperReload.runSpeedToPosition(); 
  }
}

void run_drill_forward(float seconds, float tenth_seconds){
  for(j = 0;j<=seconds;j++){//this is the seconds
    if(j == seconds){ //if on last loop
       i_max = 32767*tenth_seconds; 
    }
    else {
       i_max = 32767; 
    }
    for(i=0;i<i_max;i++){ //33112 loops for second. 2^15 - 1 = 32767 max = 0.987s so close enough 
     digitalWrite(LPWM,HIGH);//Front, 5.1 microsec per instruction
     delayMicroseconds(15.6); //20microsecond in total per loop for the delays
     digitalWrite(LPWM,LOW);//Front
     delayMicroseconds(4.4);
    }
  }
}

void run_drill_back(float seconds, float tenth_seconds){
  for(j = 0;j<=seconds;j++){//this is the seconds
    //32767/i = 1/0.5 to find the i value, thus do i = 32767*frac to find i
    if(j == seconds){ //if on last loop
       i_max = 32767*tenth_seconds; 
    }
    else {
       i_max = 32767; 
    }
    for(i=0;i<i_max;i++){ //33112 loops for second. 2^15 - 1 = 32767 max = 0.987s so close enough 
     digitalWrite(RPWM,HIGH);//Front, 5.1 microsec per instruction
     delayMicroseconds(15.6); //20microsecond in total per loop for the delays
     digitalWrite(RPWM,LOW);//Front
     delayMicroseconds(4.4);
    }
  }
}

void resetData(){
  int i = 0;
  while(i<max_length){
    test[i] = '\0'; // \0 means an empty array index, this will be used to find the end of the parsed command
    i = i + 1; 
  }
  i = 0;
  while(i<max_length){
    var_1[i] = '\0'; // \0 means an empty array index, this will be used to find the end of the parsed command
    var_2[i] = '\0';
    var_3[i] = '\0';
    i = i + 1; 
  }
}
