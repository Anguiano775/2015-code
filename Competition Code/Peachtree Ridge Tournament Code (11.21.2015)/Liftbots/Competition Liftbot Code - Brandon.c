#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in8,    gyro,           sensorGyro)
#pragma config(Sensor, dgtl1,  PneuF1,         sensorDigitalOut)
#pragma config(Sensor, dgtl2,  PneuF2,         sensorDigitalOut)
#pragma config(Sensor, dgtl3,  PneuB1,         sensorDigitalOut)
#pragma config(Sensor, dgtl4,  PneuB2,         sensorDigitalOut)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Sensor, I2C_3,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Sensor, I2C_4,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Motor,  port1,           LBdrive,       tmotorVex393_HBridge, openLoop, driveLeft, encoderPort, I2C_4)
#pragma config(Motor,  port2,           LFdrive,       tmotorVex393_MC29, openLoop, driveLeft, encoderPort, I2C_1)
#pragma config(Motor,  port3,           ramp1,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           ramp2,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           flywheel1,     tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port6,           flywheel2,     tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port7,           intake1,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           intake2,       tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           RFdrive,       tmotorVex393_MC29, openLoop, reversed, driveRight, encoderPort, I2C_2)
#pragma config(Motor,  port10,          RBdrive,       tmotorVex393_HBridge, openLoop, reversed, driveRight, encoderPort, I2C_3)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

#include "Vex_Competition_Includes.c"
//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(15)
#pragma userControlDuration(105)
#define FORWARD 1
#define BACKWARD 0
#define RIGHT 1
#define LEFT 0
#define RAISE 1
#define LOWER 0
const short leftButton = 1;
const short centerButton = 2;
const short rightButton = 4;
int count = 0;
int OoT = 0;
int BoR = 0;
int LCDselect = 1;


void turnRight(int amount,int power)
{
	SensorValue[gyro] = 0;
	while(abs(SensorValue[gyro]) < amount)
	{
		motor[RBdrive] = power;
		motor[RFdrive] = power;
		motor[LFdrive] = -power;
		motor[LBdrive] = -power;
	}
	motor[RBdrive] = 0;
	motor[RFdrive] = 0;
	motor[LFdrive] = 0;
	motor[LBdrive] = 0;
	SensorValue[gyro] = 0;
}
void turnLeft(int amount,int power)
{
	SensorValue[gyro] = 0;
	while(abs(SensorValue[gyro]) < amount)
	{
		motor[RBdrive] = -power;
		motor[RFdrive] = -power;
		motor[LFdrive] = power;
		motor[LBdrive] = power;
	}
	motor[RBdrive] = 0;
	motor[RFdrive] = 0;
	motor[LFdrive] = 0;
	motor[LBdrive] = 0;
	SensorValue[gyro] = 0;
}

void moveForward(int amount,int power)
{
	nMotorEncoder[LFdrive] = 0;
	while(abs(nMotorEncoder[LFdrive]) < amount)
	{
		motor[RBdrive] = power;
		motor[RFdrive] = power;
		motor[LFdrive] = power;
		motor[LBdrive] = power;

	}
	motor[RBdrive] = 0;
	motor[RFdrive] = 0;
	motor[LFdrive] = 0;
	motor[LBdrive] = 0;
	nMotorEncoder[LFdrive] = 0;
}
void moveBackward(int amount,int power)
{
	nMotorEncoder[LFdrive] = 0;
	while(abs(nMotorEncoder[LFdrive]) < amount)
	{
		motor[RBdrive] = -power;
		motor[RFdrive] = -power;
		motor[LFdrive] = -power;
		motor[LBdrive] = -power;
	}
	motor[RBdrive] = 0;
	motor[RFdrive] = 0;
	motor[LFdrive] = 0;
	motor[LBdrive] = 0;
	nMotorEncoder[LFdrive] = 0;
}
task lautonomousCodew()
{
	wait10Msec(500);
  moveBackward(3360,50); 	// moves to goal

  wait10Msec(50);
  turnLeft(150,50);				//faces goal
  wait10Msec(50);


  motor[flywheel1] = -127;//starts flywheel
  motor[flywheel2] = -127;


  wait10Msec(200);
  motor[intake1] = -117;
  motor[intake2] = -117;		//launches 1st ball
  wait10Msec(50);
  motor[intake1] = 0;
  motor[intake2] = 0;


  wait10Msec(50);
  motor[intake1] = -117;		//lauches 2nd ball
  motor[intake2] = -117;
  wait10Msec(50);
  motor[intake1] = 0;
  motor[intake2] = 0;



}
task lautonomousCode()
{

  moveBackward(3360,50); 	// moves to goal 5260 3360

  wait10Msec(50);
  turnLeft(150,50);				//faces goal
  wait10Msec(50);


  motor[flywheel1] = -127;//starts flywheel
  motor[flywheel2] = -127;


  wait10Msec(150);
  motor[intake1] = -107;
  motor[intake2] = -107;		//launches 1st ball
  wait10Msec(50);
  motor[intake1] = 0;
  motor[intake2] = 0;


  wait10Msec(50);
  motor[intake1] = -107;		//lauches 2nd ball
  motor[intake2] = -107;
  wait10Msec(50);
  motor[intake1] = 0;
  motor[intake2] = 0;
}
task rautonomousCode()
{

  moveBackward(3360,50); 	// moves to goal

  wait10Msec(50);
  turnRight(150,50);				//faces goal
  wait10Msec(50);


  motor[flywheel1] = -127;//starts flywheel
  motor[flywheel2] = -127;


  wait10Msec(200);
  motor[intake1] = -117;
  motor[intake2] = -117;		//launches 1st ball
  wait10Msec(50);
  motor[intake1] = 0;
  motor[intake2] = 0;


  wait10Msec(50);
  motor[intake1] = -117;		//lauches 2nd ball
  motor[intake2] = -117;
  wait10Msec(50);
  motor[intake1] = 0;
  motor[intake2] = 0;


}

task rautonomousCodew()
{
	wait10Msec(500);
  moveBackward(3360,50); 	// moves to goal

  wait10Msec(50);
  turnRight(150,50);				//faces goal
  wait10Msec(50);


}
void waitForRelease(){
	while(nLCDButtons != 0){}
	wait1Msec(5);
}
task LCD()
{
	//Declare count variable to keep track of our choice
	int count = 0;
	LCDselect = 0;
	//------------- Beginning of User Interface Code ---------------
	//Clear LCD
	clearLCDLine(0);
	clearLCDLine(1);
	//Loop while center button is not pressed
	while(LCDselect == 0 && nSysTime < 20000)
	{
		//Switch case that allows the user to choose from 4 different options
		while(count == 0 && nSysTime < 20000){
			//Display first choice
			displayLCDCenteredString(0, "No Wait Left");
			displayLCDCenteredString(1, "<		 Enter		>");
			//Increment or decrement "count" based on button press
			if(nLCDButtons == leftButton)
			{
				waitForRelease();
				count = 3;
			}
			else if(nLCDButtons == rightButton)
			{
				waitForRelease();
				count++;
			}
			else if(nLCDButtons == centerButton){
				OoT = 1;
				BoR = 1;
				LCDselect = 1;
				break;
				waitForRelease();

			}
		}
		while(count == 1 && nSysTime < 20000){
			//Display first choice
			displayLCDCenteredString(0, "5 Second Wait left");
			displayLCDCenteredString(1, "<		 Enter		>");
			//Increment or decrement "count" based on button press
			if(nLCDButtons == leftButton)
			{
				waitForRelease();
				count--;
			}
			else if(nLCDButtons == rightButton)
			{
				waitForRelease();
				count++;
			}
			else if(nLCDButtons == centerButton){
				OoT = 2;
				BoR = 1;
				LCDselect = 1;
				break;
				waitForRelease();

			}
	}
	while(count == 2 && nSysTime < 20000){
			//Display first choice
			displayLCDCenteredString(0, "No Wait Right");
			displayLCDCenteredString(1, "<		 Enter		>");
			//Increment or decrement "count" based on button press
			if(nLCDButtons == leftButton)
			{
				waitForRelease();
				count--;
			}
			else if(nLCDButtons == rightButton)
			{
				waitForRelease();
				count++;
			}
			else if(nLCDButtons == centerButton){
				OoT = 1;
				BoR = 2;
				LCDselect = 1;
				break;
				waitForRelease();

			}
	}
	while(count == 3 && nSysTime < 20000){
			//Display first choice
			displayLCDCenteredString(0, "5 Second Wait Right");
			displayLCDCenteredString(1, "<		 Enter		>");
			//Increment or decrement "count" based on button press
			if(nLCDButtons == leftButton)
			{
				waitForRelease();
				count--;
			}
			else if(nLCDButtons == rightButton)
			{
				waitForRelease();
				count = 0;
			}
			else if(nLCDButtons == centerButton){
				OoT = 2;
				BoR = 2;
				LCDselect = 1;
				break;
				waitForRelease();

			}
	}
	}
}
/*task LCD()
{
	//Declare count variable to keep track of our choice
	int screen = 0;
	int selected = 0;
	LCDselect = 0;
	//------------- Beginning of User Interface Code ---------------
	//Clear LCD
	clearLCDLine(0);
	clearLCDLine(1);
	//Loop while center button is not pressed
	while(LCDselect == 0)
	{
		//Switch case that allows the user to choose from 4 different options
		while(LCDselect == 0){
			//Display first choice
			displayLCDChar(0, 5, time);
			//Increment or decrement "count" based on button press
			if(nLCDButtons == leftButton)
			{
				if(time >= 0)
				{
				waitForRelease();
				time--;
			}
			}
			else if(nLCDButtons == rightButton)
			{
				waitForRelease();
				time++;
			}
			else if(nLCDButtons == centerButton){
				LCDselect = 1;
				selected++;
				break;
				waitForRelease();
				count = 1;
			}
		}
	}
	clearLCDLine(0);
	clearLCDLine(1);
}
*/
//------------- End of User Interface Code ---------------------

//------------- Beginning of Robot Movement Code ---------------
//Clear LCD
//Switch Case that actually runs the user choice



//------------- End of Robot Movement Code -----------------------



void LCDselection(){

	if(OoT == 1 && BoR == 1){
		startTask(lautonomousCode);
		wait1Msec(15000);
		stopTask(lautonomousCode);
	}
	if(OoT == 1 && BoR == 2)
	{
		startTask(lautonomousCodew);
		wait1Msec(15000);
		stopTask(lautonomousCodew);
	}
	if(OoT == 2 && BoR == 1)
	{
		startTask(rautonomousCode);
		wait1Msec(15000);
		stopTask(rautonomousCode);
	}
	if(OoT == 2 && BoR == 2)
	{
		startTask(rautonomousCodew);
		wait1Msec(15000);
		stopTask(rautonomousCodew);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
//
//                          Pre-Autonomous Functions
//
// You may want to perform some actions before the competition starts. Do them in the
// following function.
//
/////////////////////////////////////////////////////////////////////////////////////////

void pre_auton()
{
  bLCDBacklight=true;
	bStopTasksBetweenModes = true;
	SensorValue[I2C_2] = 0;
	nMotorEncoder[RFdrive] = 0;
	// All activities that occur before the competition starts
	// Example: clearing encoders, setting servo positions, ...
	startTask(LCD);
	// All activities that occur before the competition starts
	// Example: clearing encoders, setting servo positions, ...
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//                                 Autonomous Task
//
// This task is used to control your robot during the autonomous phase of a VEX Competition.
// You must modify the code to add your own robot specific commands here.
//
/////////////////////////////////////////////////////////////////////////////////////////

task autonomous()
{
	LCDselection();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//                                 User Control Task
//
// This task is used to control your robot during the user control phase of a VEX Competition.
// You must modify the code to add your own robot specific commands here.
//
/////////////////////////////////////////////////////////////////////////////////////////

task usercontrol()
{
	// User control code here, inside the loop
  while(1 == 1)
  {
  	motor[LBdrive] = vexRT[Ch3];		//sets drive motor speeds to joysticks
  	motor[LFdrive] = vexRT[Ch3];
  	motor[RBdrive] = vexRT[Ch2];
  	motor[RFdrive] = vexRT[Ch2];

    if(vexRT[Btn5U] == 1)						//intakes balls
    {
    	motor[intake1] = 117;
    	motor[intake2] = 117;
    }
    else if (vexRT[Btn5D] == 1)					//outputs balls
    {
    	motor[intake1] = -117;
    	motor[intake2] = -117;
    }
    else														//turns off intake
    {
    	motor[intake1] = 0;
    	motor[intake2] = 0;
    }

    if(vexRT[Btn8D])						//lowers ramp
    {
    	motor[ramp1] = 110;
    	motor[ramp2] = 110;
    }
    else if(vexRT[Btn8U])						//pulls in ramp pushers
    {
    	motor[ramp1] = -110;
    	motor[ramp2] = -110;
    }
    else
    {
    	motor[ramp1] = 0;
    	motor[ramp2] = 0;
    }
    if(vexRT[Btn6D] == 1)
    {
    	motor[flywheel1] = 127;
    	motor[flywheel2] = 127;
    }
    else if(vexRT[Btn6U] == 1)
    {
    	motor[flywheel2] = -127;
    	motor[flywheel1] = -127;
    }
    else
    {
    	motor[flywheel1] = 0;
    	motor[flywheel2] = 0;
    }
    if(vexRT[Btn7U] == 1)
    {
    	SensorValue[PneuF1] = 1;
    	SensorValue[PneuF2] = 1;
    	SensorValue[PneuB1] = 1;
    	SensorValue[PneuB2] = 1;
    }
		else if(vexRT[Btn7D] == 1)
		{
			SensorValue[PneuF1] = 1;
			SensorValue[PneuF2] = 1;
		}
    else
    {
    	SensorValue[PneuF1] = 0;
    	SensorValue[PneuF2] = 0;
    	SensorValue[PneuB1] = 0;
    	SensorValue[PneuB2] = 0;
   	}
    if(!bVEXNETActive)
   	{
   		SensorValue[PneuF1] = 0;
    	SensorValue[PneuF2] = 0;
    	SensorValue[PneuB1] = 0;
    	SensorValue[PneuB2] = 0;
  	}

   }
}