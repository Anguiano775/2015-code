#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in1,    gyro,           sensorGyro)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_3,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_4,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           intakeRoller,  tmotorVex393TurboSpeed_HBridge, openLoop)
#pragma config(Motor,  port2,           rFlyTop,       tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port3,           rFlyBottom,    tmotorVex393HighSpeed_MC29, openLoop, reversed, encoderPort, I2C_2)
#pragma config(Motor,  port4,           rDriveFront,   tmotorVex393HighSpeed_MC29, openLoop, reversed, encoderPort, I2C_4)
#pragma config(Motor,  port5,           intakeChain,   tmotorVex393HighSpeed_MC29, openLoop, reversed, encoderPort, I2C_3)
#pragma config(Motor,  port6,           lDriveBack,    tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port7,           lDriveFront,   tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port8,           lFlyTop,       tmotorVex393HighSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           lFlyBottom,    tmotorVex393HighSpeed_MC29, openLoop, encoderPort, I2C_1)
#pragma config(Motor,  port10,          rDriveBack,    tmotorVex393TurboSpeed_HBridge, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!
#include "..\Global\Simple PID Controller.h"

fw_controller lFly, rFly;
string str;

void pre_auton()
{
	// Set bStopTasksBetweenModes to false if you want to keep user created tasks running between
	// Autonomous and Tele-Op modes. You will need to manage all user created tasks if set to false.
	bStopTasksBetweenModes = true;

	SensorType[gyro] = sensorNone;
  wait1Msec(500);
  //Reconfigure Analog Port 8 as a Gyro sensor and allow time for ROBOTC to calibrate it
  SensorType[gyro] = sensorGyro;
  wait1Msec(2000);
  displayLCDCenteredString(0,"");

}

void setLDriveMotors (float power) {
	motor[lDriveFront] = power;
	motor[lDriveBack] = power;
}

void setRDriveMotors (float power) {
	motor[rDriveFront] = power;
	motor[rDriveBack] = power;
}

void setIntakeMotors (float power) {
	motor[intakeChain] = power;
	motor[intakeRoller] = power;
}

//work in progress
void driveDistance (int encoderCounts, int direction, float power) {
	int encoderGoalRight = nMotorEncoder[rDriveFront] + encoderCounts*direction;

	while (nMotorEncoder[rDriveFront] < encoderGoalRight) {
		setLDriveMotors(power*direction);
		setRDriveMotors(power*direction);
		wait1Msec(10);
	}
	setLDriveMotors(-10);
	setRDriveMotors(-10);
	wait1Msec(125);
	setLDriveMotors(0);
	setRDriveMotors(0);
}

//rotate the robot to a certain position (rotationally)
//@param deg The number of degrees to turn; positive values are counterclockwise, negative values are clockwise.
//@param direction The direction to turn in to get to the position; 1 is counterclockwise, -1 is clockwise
void rotate (int deg, int direction) {
	//Specify the number of degrees for the robot to turn (1 degree = 10, or 900 = 90 degrees)
  int degrees10 = deg*10; //multiply the degrees parameter by 10 to get the amount to turn relative to gyro sensor values

  //Specify the amount of acceptable error in the turn
  int error = 5;

  //While the absolute value of the gyro is less than the desired rotation - 100...
  while(abs(SensorValue[gyro]) < degrees10 - 100)
  {
    setRDriveMotors(65*direction);
    setLDriveMotors(-65*direction);
  }
  //Brief brake to eliminate some drift
  setRDriveMotors(-5*direction);
  setLDriveMotors(5*direction);
  wait1Msec(100);

  //Second while loop to move the robot more slowly to its goal, also setting up a range
  //for the amount of acceptable error in the system
  while(abs(SensorValue[gyro]) > degrees10 + error || abs(SensorValue[gyro]) < degrees10 - error)
  {
    if(abs(SensorValue[gyro]) > degrees10)
    {
      setRDriveMotors(-40);
      setLDriveMotors(40);
    }
    else
    {
      setRDriveMotors(40);
      setLDriveMotors(-40);
    }
  }

  //Stop
	setRDriveMotors(0);
	setLDriveMotors(0);
}


void setLeftFwSpeed (float power) {
	motor[lFlyTop] = power;
	motor[lFlyBottom] = power;
}

void setRightFwSpeed (float power) {
	motor[rFlyTop] = power;
	motor[rFlyBottom] = power;
}

/*-----------------------------------------------------------------------------*/
/** @brief     Task to control the velocity of the left flywheel               */
/*-----------------------------------------------------------------------------*/
task leftFwControlTask()
{
	fw_controller *fw = lFly;

	// We are using Speed geared motors
	// Set the encoder ticks per revolution
	fw->ticks_per_rev = fw->MOTOR_TPR;

	while(1)
	{
		// debug counter
		fw->counter++;

		// Calculate velocity
		getEncoderAndTimeStamp(lFlyBottom,fw->e_current, fw->encoder_timestamp);
		FwCalculateSpeed(fw);

		// Set current speed for the tbh calculation code

		fw->v_current = getMotorVelocity(lFlyBottom);
		fw->current = fw->v_current;

		// Do the velocity TBH calculations
		FwControlUpdateVelocityTbh( fw ) ;

		// Scale drive into the range the motors need
		fw->motor_drive  = fw->drive * (FW_MAX_POWER/127);

		// Final Limit of motor values - don't really need this
		if( fw->motor_drive >  127 ) fw->motor_drive =  127;
		if( fw->motor_drive < -127 ) fw->motor_drive = -127;

		// and finally set the motor control value
		//if(fw->current < fw->target - 20) {
		//	setLeftFwSpeed( 70 );
		//} else
			setLeftFwSpeed(fw->motor_drive);
		str = sprintf( str, "%4d %4d  %5.2f", fw->target,  fw->current, nImmediateBatteryLevel/1000.0 );
		displayLCDString(0, 0, str );
		str = sprintf( str, "%4.2f %4.2f ", fw->drive, fw->drive_at_zero );
		displayLCDString(1, 0, str );
		// Run at somewhere between 20 and 50mS
		wait1Msec( FW_LOOP_SPEED );
	}
}

/*-----------------------------------------------------------------------------*/
/** @brief     Task to control the velocity of the right flywheel              */
/*-----------------------------------------------------------------------------*/
task rightFwControlTask()
{
	fw_controller *fw = rFly;

	// We are using Speed geared motors
	// Set the encoder ticks per revolution
	fw->ticks_per_rev = fw->MOTOR_TPR;

	while(1)
	{
		// debug counter
		fw->counter++;

		// Calculate velocity
		getEncoderAndTimeStamp(rFlyBottom,fw->e_current, fw->encoder_timestamp);
		FwCalculateSpeed(fw);

		// Set current speed for the tbh calculation code
		fw->current = fw->v_current;

		// Do the velocity TBH calculations
		FwControlUpdateVelocityTbh( fw ) ;

		// Scale drive into the range the motors need
		fw->motor_drive  = fw->drive * (FW_MAX_POWER/127);

		// Final Limit of motor values - don't really need this
		if( fw->motor_drive >  127 ) fw->motor_drive =  127;
		if( fw->motor_drive < -127 ) fw->motor_drive = -127;

		// and finally set the motor control value
		//if(fw->current < fw->target - 20) {
		//	setRightFwSpeed( 70 );
		//} else
			setRightFwSpeed( fw->motor_drive );

		// Run at somewhere between 20 and 50mS
		wait1Msec( FW_LOOP_SPEED );
	}
}

//long shooting
void initializePIDLong() {
	//tbhInit(lFly, 392, 0.6, 0.008064, 0, 70); //initialize PID for left side of the flywheel
	//tbhInit(rFly, 392, 0.55, 0.008064, 0, 70); //initialize PID for right side of the flywheel
	//note the order of the parameters:
	//(controller, motor ticks per rev, KpNorm, KpBallLaunch, Ki, Kd, constant, RPM drop on ball launch)
	tbhInit(lFly, 392, 0.2481, 0.6000, 0.005481, 0, 75, 20); //initialize PID for left side of the flywheel //left side might be able to have a higher P
	tbhInit(rFly, 392, 0.2481, 0.6000, 0.005481, 0, 75, 20); //initialize PID for right side of the flywheel //x.x481
	startTask(leftFwControlTask);
	startTask(rightFwControlTask);
}

//short shooting
void initializePIDShort() {
	//note the order of the parameters:
	//(controller, motor ticks per rev, KpNorm, KpBallLaunch, Ki, Kd, constant, RPM drop on ball launch)
	tbhInit(lFly, 392, 0.7481, 0.8481, 0.005481, 0, 50, 20); //initialize PID for left side of the flywheel //left side might be able to have a higher P
	tbhInit(rFly, 392, 0.7481, 0.8481, 0.005481, 0, 50, 20); //initialize PID for right side of the flywheel //x.x481
	startTask(leftFwControlTask);
	startTask(rightFwControlTask);
}
void initializePIDPurple() {
	//note the order of the parameters:
	//(controller, motor ticks per rev, KpNorm, KpBallLaunch, Ki, Kd, constant, RPM drop on ball launch)
	tbhInit(lFly, 392, 0.6681, 1.36, 0.005481, 0, 55, 20); //initialize PID for left side of the flywheel //left side might be able to have a higher P
	tbhInit(rFly, 392, 0.6681, 1.36, 0.005481, 0, 55, 20); //initialize PID for right side of the flywheel //x.x481
	startTask(leftFwControlTask);
	startTask(rightFwControlTask);
}
void stopFlywheel() {
	//disable PIC control of the flywheels
	stopTask(leftFwControlTask);
	stopTask(rightFwControlTask);
	//turn off the flywheel motors
	setLeftFwSpeed(0);
	setRightFwSpeed(0);
}
task autonomous()
{
	//blue side
	initializePIDShort();
	FwVelocitySet(lFly, 83, .5); //Added For Short Shot Test -- Crawford
	FwVelocitySet(rFly, 83, .5); //Added For Short Shot Test -- Crawford
	driveDistance(3450, 1, 125);
	wait1Msec(500);
	setIntakeMotors(122);
	//rotate(37.1,1);
}

int lSpeed = 55; //Added For Short Shot Test -- Crawford
int rSpeed = 55; //Added For Short Shot Test -- Crawford
//int lSpeed = 70; // Evan's Long Shot
//int rSpeed = 70; // Evan's Long Shot
int flywheelWorking = 0;
task usercontrol()
{
	//startTask(autonomous);
	//writeDebugStreamLine("nPgmTime,lFly.current, lFly.motor_drive, lFly.p, lFly.i, lFly.d, lFly.constant, 50*lFly.postBallLaunch, rFly.current, rFly.motor_drive, rFly.p, rFly.i, rFly.d, rFly.constant, 60*rFly.postBallLaunch");
	//setLeftFwSpeed(lSpeed);
	//setRightFwSpeed(rSpeed);
	//wait1Msec(500);

	//short shooting
	//initializePIDShort();
	//FwVelocitySet(lFly, 83, .5); //Added For Short Shot Test -- Crawford
	//FwVelocitySet(rFly, 83, .5); //Added For Short Shot Test -- Crawford

	//long shooting
	//initializePIDLong();
	//FwVelocitySet(lFly,150,.7); // Evan's Long Shot 141
	//FwVelocitySet(rFly,150,.7); // Evan's Long Shot 141

	//short shooting
	int threshold = 15,
	lY,
	rY;
	while (true)
	{
		lY = vexRT[Ch3]*2;
		rY = vexRT[Ch2]*2;
		motor[lDriveFront] = (abs(lY) > threshold) ? lY : 0;
		motor[lDriveBack] = (abs(lY) > threshold) ? lY : 0;
		motor[rDriveFront] = (abs(rY) > threshold) ? rY : 0;
		motor[rDriveBack] = (abs(rY) > threshold) ? rY : 0;
		if(vexRT[Btn7L] == 1)
		{
			flywheelWorking = 1;
			initializePIDLong();
			FwVelocitySet(lFly,136,.7); // Long Shot
			FwVelocitySet(rFly,136,.7); // Long Shot
			motor[intakeChain] = 127;
			motor[intakeRoller] = 127;
	  }
	  else if(vexRT[Btn7R] == 1)
	  {
	  	initializePIDLong();
			FwVelocitySet(lFly,136,.7); // Long Shot
			FwVelocitySet(rFly,136,.7); // Long Shot
	  }
	  else if(vexRT[Btn7U] == 1)
	  {
	  	flywheelWorking = 1;
	  	initializePIDPurple();
			FwVelocitySet(lFly, 120, .5);
			FwVelocitySet(rFly, 120, .5);
		}
	  else if(vexRT[Btn7D] == 1)
	  {
	  	initializePIDShort();
	    FwVelocitySet(lFly, 83, .5); //Added For Short Shot Test -- Crawford
	    FwVelocitySet(rFly, 83, .5); //Added For Short Shot Test -- Crawford
	  }
	  else if(vexRT[Btn8D] == 1)
	  {
	  	flywheelWorking = 0;
	  	stopFlywheel();
	  }
	  if(vexRT[Btn5D] == 1)
	  {
	  	motor[intakeRoller] = 127;
	  }
	  else if(vexRT[Btn5U] == 1)
	  {
	  	motor[intakeRoller] = -127;
	  }
	  else if(flywheelWorking == 0)
	  {
	  	motor[intakeRoller] = 0;
	  }
	  if(vexRT[Btn6D] == 1)
	  {
	  	motor[intakeChain] = 127;
	  }
	  else if(vexRT[Btn6U] == 1)
	  {
	  	motor[intakeChain] = -127;
	  }
	  else if(flywheelWorking == 0)
	  {
	  	motor[intakeChain] = 0;
	  }
}
}
