#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, dgtl3,  readyLED,       sensorLEDtoVCC)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Motor,  port1,           intake1,       tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           Fly1,          tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           Fly2,          tmotorVex393_MC29, openLoop, reversed, encoderPort, I2C_1)
#pragma config(Motor,  port4,           Fly3,          tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           rightDrive,    tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           leftDrive,     tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port7,           Fly4,          tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           Fly5,          tmotorVex393_MC29, openLoop, encoderPort, I2C_2)
#pragma config(Motor,  port9,           Fly6,          tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port10,          intake2,       tmotorVex393_HBridge, openLoop)
#pragma platform(VEX)

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


#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!
int OoT = 0;
int BoR = 0;
int LCDselect = 1;
void waitForRelease(){
	while(nLCDButtons != 0)
	wait1Msec(5);
}
//attribution for our take-back-half algorithm.  The version present in this code
//is very similar to that found in the oriignal version of the algorithm, save
//that the gains have been changed and the code has been adapted to work with both
//sides of our flywheel.
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2015                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     flywheel_s.c                                                 */
/*    Author:     James Pearman                                                */
/*    Created:    28 June 2015                                                 */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00  28 June 2015 - Initial release    										 */

//Global variables
bool flywheelRunning = false; //global variable to keep track of flywheel status - this will allow us to prevent the acceleration function from being called more than once

// Update inteval (in mS) for the flywheel control loop
#define FW_LOOP_SPEED              50

// Maximum power we want to send to the flywheel motors
#define FW_MAX_POWER              127

// encoder counts per revolution depending on motor
#define LCLICKS_PER_REV         606
#define RCLICKS_PER_REV					623

//left side
// encoder tick per revolution
float           l_ticks_per_rev;          ///< encoder ticks per revolution

// Encoder
long            l_encoder_counts;         ///< current encoder count
long            l_encoder_counts_last;    ///< current encoder count

// velocity measurement
float           l_motor_velocity;         ///< current velocity in rpm
long            l_nSysTime_last;          ///< Time of last velocity calculation

// TBH control algorithm variables
long            l_target_velocity;        ///< target_velocity velocity
float           l_current_error;          ///< error between actual and target_velocity velocities
float           l_last_error;             ///< error last time update called
float           gain;                 	  ///< gain
float           l_drive;                  ///< final drive out of TBH (0.0 to 1.0)
float           l_drive_at_zero;          ///< drive at last zero crossing
long            l_first_cross;            ///< flag indicating first zero crossing
float           l_drive_approx;           ///< estimated open loop drive

// final motor drive
long            l_motor_drive;            ///< final motor control value

//right side
// encoder tick per revolution
float           r_ticks_per_rev;          ///< encoder ticks per revolution
// Encoder
long            r_encoder_counts;         ///< current encoder count
long            r_encoder_counts_last;    ///< current encoder count

// velocity measurement
float           r_motor_velocity;         ///< current velocity in rpm
long            r_nSysTime_last;          ///< Time of last velocity calculation

// TBH control algorithm variables
long            r_target_velocity;        ///< target_velocity velocity
float           r_current_error;          ///< error between actual and target_velocity velocities
float           r_last_error;             ///< error last time update called
float           r_drive;                  ///< final drive out of TBH (0.0 to 1.0)
float           r_drive_at_zero;          ///< drive at last zero crossing
long            r_first_cross;            ///< flag indicating first zero crossing
float           r_drive_approx;           ///< estimated open loop drive

// final motor drive
long            r_motor_drive;            ///< final motor control value

/*-----------------------------------------------------------------------------*/
/** @brief      Set the flywheen motors                                        */
/** @param[in]  value motor control value                                      */
/*-----------------------------------------------------------------------------*/
void
leftFwMotorSet( int value )
{
	motor[ Fly1 ] = value;
	motor[ Fly2 ] = value;
	motor[ Fly3 ] = value;
	writeDebugStreamLine("%f",value);
	//motor[ Fly4 ] = value;
	//motor[ Fly5 ] = value;
	//motor[ Fly6 ] = value;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get the flywheen motor encoder count                           */
/*-----------------------------------------------------------------------------*/
long
leftFwMotorEncoderGet()
{
	return( nMotorEncoder[ Fly2 ] );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set the controller position                                    */
/** @param[in]  desired velocity                                               */
/** @param[in]  predicted_drive estimated open loop motor drive                */
/*-----------------------------------------------------------------------------*/
void
leftFwVelocitySet( int velocity, float predicted_drive )
{
	// set target_velocity velocity (motor rpm)
	l_target_velocity = velocity;

	// Set error so zero crossing is correctly detected
	l_current_error = l_target_velocity - l_motor_velocity;
	l_last_error    = l_current_error;

	// Set predicted open loop drive value
	l_drive_approx  = predicted_drive;
	// Set flag to detect first zero crossing
	l_first_cross   = 1;
	// clear tbh variable
	l_drive_at_zero = 0;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Calculate the current flywheel motor velocity                  */
/*-----------------------------------------------------------------------------*/
void
leftFwCalculateSpeed()
{
	int     delta_ms;
	int     delta_enc;

	// Get current encoder value
	l_encoder_counts = leftFwMotorEncoderGet();

	// This is just used so we don't need to know how often we are called
	// how many mS since we were last here
	delta_ms = nSysTime - l_nSysTime_last;
	l_nSysTime_last = nSysTime;

	// Change in encoder count
	delta_enc = (l_encoder_counts - l_encoder_counts_last);

	// save last position
	l_encoder_counts_last = l_encoder_counts;

	// Calculate velocity in rpm
	l_motor_velocity = (1000.0 / delta_ms) * delta_enc * 60.0 / l_ticks_per_rev;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Update the velocity tbh controller variables                   */
/*-----------------------------------------------------------------------------*/
void
leftFwControlUpdateVelocityTbh()
{
	// calculate error in velocity
	// target_velocity is desired velocity
	// current is measured velocity
	l_current_error = l_target_velocity - l_motor_velocity;

	// Calculate new control value
	l_drive =  l_drive + (l_current_error * gain);

	// Clip to the range 0 - 1.
	// We are only going forwards
	if( l_drive > 1 )
		l_drive = 1;
	if( l_drive < 0 )
		l_drive = 0;

	// Check for zero crossing
	if( sgn(l_current_error) != sgn(l_last_error) ) {
		// First zero crossing after a new set velocity command
		if( l_first_cross ) {
			// Set drive to the open loop approximation
			l_drive = l_drive_approx;
			l_first_cross = 0;
		}
		else
			l_drive = 0.5 * ( l_drive + l_drive_at_zero );

		// Save this drive value in the "tbh" variable
		l_drive_at_zero = l_drive;
	}

	// Save last error
	l_last_error = l_current_error;
}

/*-----------------------------------------------------------------------------*/
/** @brief     Task to control the velocity of the flywheel                    */
/*-----------------------------------------------------------------------------*/
task
leftFwControlTask()
{
	// Set the gain
	gain = 0.0006;

	// We are using Speed geared motors
	// Set the encoder ticks per revolution
	l_ticks_per_rev = LCLICKS_PER_REV;

	while(1)
	{
		// Calculate velocity
		leftFwCalculateSpeed();

		// Do the velocity TBH calculations
		leftFwControlUpdateVelocityTbh() ;

		// Scale drive into the range the motors need
		l_motor_drive  = (l_drive * FW_MAX_POWER) + 0.5;

		// Final Limit of motor values - don't really need this
		if( l_motor_drive >  127 ) l_motor_drive =  127;
		if( l_motor_drive < -127 ) l_motor_drive = -127;

		// and finally set the motor control value
		leftFwMotorSet( l_motor_drive );

		// Run at somewhere between 20 and 50mS
		wait1Msec( FW_LOOP_SPEED );
	}
}


//right side functions
/*-----------------------------------------------------------------------------*/
/** @brief      Set the flywheen motors                                        */
/** @param[in]  value motor control value                                      */
/*-----------------------------------------------------------------------------*/
void
rightFwMotorSet( int value )
{
	motor[ Fly4 ] = value;
	motor[ Fly5 ] = value;
	motor[ Fly6 ] = value;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get the flywheen motor encoder count                           */
/*-----------------------------------------------------------------------------*/
long
rightFwMotorEncoderGet()
{
	return( nMotorEncoder[ Fly5 ] );
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set the controller position                                    */
/** @param[in]  desired velocity                                               */
/** @param[in]  predicted_drive estimated open loop motor drive                */
/*-----------------------------------------------------------------------------*/
void
rightFwVelocitySet( int velocity, float predicted_drive )
{
	// set target_velocity velocity (motor rpm)
	r_target_velocity = velocity;

	// Set error so zero crossing is correctly detected
	r_current_error = r_target_velocity - r_motor_velocity;
	r_last_error    = r_current_error;

	// Set predicted open loop drive value
	r_drive_approx  = predicted_drive;
	// Set flag to detect first zero crossing
	r_first_cross   = 1;
	// clear tbh variable
	r_drive_at_zero = 0;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Calculate the current flywheel motor velocity                  */
/*-----------------------------------------------------------------------------*/
void
rightFwCalculateSpeed()
{
	int     delta_ms;
	int     delta_enc;

	// Get current encoder value
	r_encoder_counts = rightFwMotorEncoderGet();

	// This is just used so we don't need to know how often we are called
	// how many mS since we were last here
	delta_ms = nSysTime - r_nSysTime_last;
	r_nSysTime_last = nSysTime;

	// Change in encoder count
	delta_enc = (r_encoder_counts - r_encoder_counts_last);

	// save last position
	r_encoder_counts_last = r_encoder_counts;

	// Calculate velocity in rpm
	r_motor_velocity = (1000.0 / delta_ms) * delta_enc * 60.0 / r_ticks_per_rev;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Update the velocity tbh controller variables                   */
/*-----------------------------------------------------------------------------*/
void
rightFwControlUpdateVelocityTbh()
{
	// calculate error in velocity
	// target_velocity is desired velocity
	// current is measured velocity
	r_current_error = r_target_velocity - r_motor_velocity;

	// Calculate new control value
	r_drive =  r_drive + (r_current_error * gain);

	// Clip to the range 0 - 1.
	// We are only going forwards
	if( r_drive > 1 )
		r_drive = 1;
	if( r_drive < 0 )
		r_drive = 0;

	// Check for zero crossing
	if( sgn(r_current_error) != sgn(r_last_error) ) {
		// First zero crossing after a new set velocity command
		if( r_first_cross ) {
			// Set drive to the open loop approximation
			r_drive = r_drive_approx;
			r_first_cross = 0;
		}
		else
			r_drive = 0.5 * ( r_drive + r_drive_at_zero );

		// Save this drive value in the "tbh" variable
		r_drive_at_zero = r_drive;
	}

	// Save last error
	r_last_error = r_current_error;
}

/*-----------------------------------------------------------------------------*/
/** @brief     Task to control the velocity of the flywheel                    */
/*-----------------------------------------------------------------------------*/
task
rightFwControlTask()
{
	// Set the gain
	gain = 0.007;

	// We are using Speed geared motors
	// Set the encoder ticks per revolution
	r_ticks_per_rev = RCLICKS_PER_REV;

	while(1)
	{
		// Calculate velocity
		rightFwCalculateSpeed();

		// Do the velocity TBH calculations
		rightFwControlUpdateVelocityTbh() ;

		// Scale drive into the range the motors need
		r_motor_drive  = (r_drive * FW_MAX_POWER) + 0.5;

		// Final Limit of motor values - don't really need this
		if( r_motor_drive >  127 ) r_motor_drive =  127;
		if( r_motor_drive < -127 ) r_motor_drive = -127;

		// and finally set the motor control value
		rightFwMotorSet( r_motor_drive );

		// Run at somewhere between 20 and 50mS
		wait1Msec( FW_LOOP_SPEED );
	}
}
//end of right side functions



//for flywheel acceleration; the separate task lets the acceleration code run concurently with other robot functions
task flywheelController() { //manages flywheel starts and stops
	while(1)
	{
		//activate/deactivate flywheel on joystick button press
		if(vexRT[Btn5D] == 1 && !flywheelRunning){
			leftFwMotorSet(40);
			rightFwMotorSet(40);
			wait1Msec(750);
			leftFwMotorSet(70);
			rightFwMotorSet(70);
			wait1Msec(750);
			leftFwMotorSet(75);
			rightFwMotorSet(75);
			wait1Msec(500);
			startTask(leftFwControlTask); //this is ok to run every time because stopping the flywheel also stops the flywheel control tasks
			startTask(rightFwControlTask);
			leftFwVelocitySet(100,1);
			rightFwVelocitySet(100,1);
		}
		else if (vexRT[Btn7D] == 1 && vexRT[Btn8D] == 1) {
			stopTask(leftFwControlTask);
			stopTask(rightFwControlTask);
			leftFwVelocitySet(0,0);
			rightFwVelocitySet(0,0);
		}
	}
}

task flywheelsForAutonomous() {
	//start flywheel control
	leftFwMotorSet(40);
	rightFwMotorSet(40);
	wait1Msec(750);
	leftFwMotorSet(70);
	rightFwMotorSet(70);
	wait1Msec(750);
			leftFwMotorSet(75);
			rightFwMotorSet(75);
			wait1Msec(500);
	startTask(leftFwControlTask);
	startTask(rightFwControlTask);

	while(1)
	{
				leftFwMotorSet(40);
				rightFwMotorSet(40);
				wait1Msec(1000);
				leftFwMotorSet(60);
				rightFwMotorSet(60);
				wait1Msec(1000);
				leftFwMotorSet(75);
				rightFwMotorSet(75);
				wait1Msec(1000);
				leftFwVelocitySet(100, 0.66);
				rightFwVelocitySet(100,0.66);


		//no else statement needed because the acceleration code looks for when it needs to stop itself
}
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
		while(nSysTime < 20000){
			//Display first choice
			displayLCDCenteredString(0, "The only autonomous play");
			displayLCDCenteredString(1, "<		 Enter		>");
			//Increment or decrement "count" based on button press
			if(nLCDButtons == leftButton)
			{
				waitForRelease();
				count = 5;
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
	clearLCDLine(0);
	clearLCDLine(1);
}
}

task autonomouscode()
{
		startTask(flywheelsForAutonomous);
		wait10Msec(600);
		motor[intake1] = 127;
		motor[intake2] = 127;
		wait10Msec(150);
		motor[intake1] = 0;
		motor[intake2] = 0;
		wait10Msec(150);
		motor[intake1] = 127;
		motor[intake2] = 127;
		wait10Msec(150);
		motor[intake1] = 0;
		motor[intake2] = 0;
		wait10Msec(150);
		motor[intake1] = 127;
		motor[intake2] = 127;
		wait10Msec(150);
		motor[intake1] = 0;
		motor[intake2] = 0;
		wait10Msec(150);
		motor[intake1] = 127;
		motor[intake2] = 127;
		wait10Msec(150);
		motor[intake1] = 0;
		motor[intake2] = 0;
		wait10Msec(150);
}
void LCDselection(){

	if(OoT == 1 && BoR == 1)
	{
		startTask(autonomouscode);
		wait1Msec(15000);
		stopTask(autonomouscode);
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
	// Set bStopTasksBetweenModes to false if you want to keep user created tasks running between
	// Autonomous and Tele-Op modes. You will need to manage all user created tasks if set to false.
	bLCDBacklight=true;
	bStopTasksBetweenModes = true;
	SensorValue[I2C_2] = 0;
	SensorValue[I2C_1] = 0;
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
	//startTask(accelerate); //controls flywheel acceleration so that the flywheel can accelerate concurrently with drivetrain and intake motors

	clearLCDLine(0);
	clearLCDLine(1);
	bLCDBacklight = true;
	char  str[32];

	bLCDBacklight = true;

	// Start the flywheel control task
	startTask(flywheelController);

	while(true)
	{
		//drivetrain

		motor[leftDrive] = vexRT[Ch3];
		motor[rightDrive] = vexRT[Ch2];

		sprintf( str, "%4d %4d  %5.2f", r_target_velocity,  r_motor_velocity, nImmediateBatteryLevel/1000.0 );
		displayLCDString(0, 0, str );
		sprintf( str, "%4.2f %4.2f ", r_drive, r_drive_at_zero );
		displayLCDString(1, 0, str );

		//intake
		if(vexRT[Btn6U] == 1)
		{
			motor[intake1] = 125;
			motor[intake2] = 125;
		}
		else if(vexRT[Btn6D] == 1)
		{
			motor[intake1] = -125;
			motor[intake2] = -125;
		}
		else
		{
			motor[intake1] = 0;
			motor[intake2] = 0;
		}
		wait1Msec(10); //so we don't overload the CPU
	}
}
