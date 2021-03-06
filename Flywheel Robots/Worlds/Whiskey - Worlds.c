#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in1,    fwBallLF,       sensorLineFollower)
#pragma config(Sensor, in2,    lowerIntakeBallLF, sensorLineFollower)
#pragma config(Sensor, dgtl1,  topGreenLED,    sensorLEDtoVCC)
#pragma config(Sensor, dgtl2,  bottomGreenLED, sensorLEDtoVCC)
#pragma config(Sensor, dgtl3,  yellowLED,      sensorLEDtoVCC)
#pragma config(Sensor, dgtl4,  redLED,         sensorLEDtoVCC)
#pragma config(Sensor, dgtl10, intakeLimit,    sensorTouch)
#pragma config(Sensor, dgtl11, yellowLED,      sensorNone)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_3,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_4,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_5,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           intakeRoller,  tmotorVex393TurboSpeed_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           lFlywheel,     tmotorVex393HighSpeed_MC29, openLoop, encoderPort, I2C_1)
#pragma config(Motor,  port3,           rDriveFront,   tmotorVex393TurboSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port4,           rDriveMiddle,  tmotorVex393TurboSpeed_MC29, openLoop, reversed, encoderPort, I2C_4)
#pragma config(Motor,  port5,           rDriveBack,    tmotorVex393TurboSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port6,           lDriveBack,    tmotorVex393TurboSpeed_MC29, openLoop)
#pragma config(Motor,  port7,           rFlyTop,       tmotorVex393TurboSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           rFlyBottom,    tmotorVex393TurboSpeed_MC29, openLoop, reversed, encoderPort, I2C_2)
#pragma config(Motor,  port9,           lDriveFrontMid, tmotorVex393TurboSpeed_MC29, openLoop, encoderPort, I2C_5)
#pragma config(Motor,  port10,          intakeChain,   tmotorVex393TurboSpeed_HBridge, openLoop, reversed, encoderPort, I2C_3)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)

#include "..\..\Vex_Competition_Includes_No_LCD.c"   //Main competition background code...do not modify!
#include "..\..\LCD Autonomous Play Selection.c"
#include "..\State\Global\Simple PID Controller.h"
//#include "Position PID.c"

fw_controller lFly, rFly;
string str;
float flywheelMode = 0; //0 - stopped, 0.5 - stopping, 1 - close, 2 - center, 3 - purple, 3.5 - field edge, 4 - long
												//  Field edge shooting is 3.5 so that things in the code dependent on the shooting mode values
												//     are not affected by this additional flywheel shooting mode
int ballsInIntake = 0,
    rollerState = 0; //-1 - backwards, 0 - stopped, 1 - forward

//variables that can affect user intaking
//userIntakeControl - whether the user can move the intake; false disables joystick control of the intake (use to programmatically control intake); true allow joystick control of intake
//outtakeOnly - when true, user can only move intake back; when false, user has full control of intake under limits of userIntakeControl
//overrideAutoIntake - when true, prevents the autoIntake task from setting userIntakeControl to true when userIntakeControl is false
    //this is used to prevent autoIntake from interfering with other tasks that set userIntakeControl to false in order to have programmatic control of the intake
		//because of the way the autoIntake task works, overrideAutoIntake *must* be set to TRUE before userIntakeControl is set to FALSE
    bool outtakeOnly = false,
		 overrideAutoIntake = false;

#define FORWARD 1;
#define BACKWARD -1;

void setLDriveMotors (float power) {
	motor[lDriveFrontMid] = power;
	motor[lDriveBack] = power;
}

void setRDriveMotors (float power) {
	motor[rDriveFront] = power;
	motor[rDriveMiddle] = power;
	motor[rDriveBack] = power;
}

void setIntakeChain (float power) {
	motor[intakeChain] = power;
}

void setIntakeRoller (float power) {
	motor[intakeRoller] = power;
	if (power > 0) {
		rollerState = 1; //moving forward
	} else if (power < 0) {
		rollerState = -1; //moving backwards
	} else {
		rollerState = 0;
	}
}

void setIntakeMotors (float power) {
	setIntakeChain(power);
	setIntakeRoller(power);
}

int yellowLEDFlashTime = 0; //the time the flashing yellow LED should stay on or off, in milliseconds.  This should be equivalent to half a period.

task flashLED() {
	while(1) {
		if  (flywheelMode < 1) {
			switch (ballsInIntake) {
				case 0:
					SensorValue[topGreenLED] = false;
					SensorValue[bottomGreenLED] = false;
					if (yellowLEDFlashTime == 0) { //only control the yellow LED if something else isn't flashing it
						SensorValue[yellowLED] = false;
					}
					SensorValue[redLED] = false;
					break;
				case 1:
					SensorValue[topGreenLED] = true;
					SensorValue[bottomGreenLED] = false;
					if (yellowLEDFlashTime == 0) { //only control the yellow LED if something else isn't flashing it
						SensorValue[yellowLED] = false;
					}
					SensorValue[redLED] = false;
					break;
				case 2:
					SensorValue[bottomGreenLED] = true;
					if (yellowLEDFlashTime == 0) { //only control the yellow LED if something else isn't flashing it
						SensorValue[yellowLED] = false;
					}
					SensorValue[redLED] = false;
					break;
				case 3:
					if (yellowLEDFlashTime == 0) { //only control the yellow LED if something else isn't flashing it
						SensorValue[yellowLED] = true;
					}
					SensorValue[redLED] = false;
					break;
				case 4:
					SensorValue[redLED] = true;
					break;
			}
		}
		if (yellowLEDFlashTime > 0) {  //only flash the LED if flash time is greater than 0
			SensorValue[yellowLED] = true;
			wait1Msec(yellowLEDFlashTime);
			SensorValue[yellowLED] = false;
			wait1Msec(yellowLEDFlashTime);
		} else {
			wait1Msec(150); //since this task isn't doing much, we can run it at a much slower frequency
		}
	}
}

void pre_auton()
{
	// Set bStopTasksBetweenModes to false if you want to keep user created tasks running between
	// Autonomous and Tele-Op modes. You will need to manage all user created tasks if set to false.
	bStopTasksBetweenModes = true;

	startTask(selectionController); //run LCD selection
}

void setLeftFwSpeed (float power) {
	motor[lFlywheel] = power;
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
		getEncoderAndTimeStamp(lFlywheel,fw->e_current, fw->encoder_timestamp);
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
		setLeftFwSpeed(fw->motor_drive);

		//output debug info on the LCD
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
		setRightFwSpeed( fw->motor_drive );

		// Run at somewhere between 20 and 50mS
		wait1Msec( FW_LOOP_SPEED );
	}
}

//long shooting
void initializePIDLong() {
	//note the order of the parameters:
	//(controller, motor ticks per rev, KpNorm, KpBallLaunch, Ki, Kd, constant, RPM drop on ball launch)
	tbhInit(lFly, 392, 0.4912, 3, 0.005481, 0, 70, 20); //initialize PID for left side of the flywheel //left side might be able to have a higher P
	tbhInit(rFly, 392, 0.4912, 3, 0.005481, 0, 70, 20); //initialize PID for right side of the flywheel //x.x481
	startTask(leftFwControlTask);
	startTask(rightFwControlTask);
}

//short shooting
void initializePIDShort() {
	//note the order of the parameters:
	//(controller, motor ticks per rev, KpNorm, KpBallLaunch, Ki, Kd, constant, RPM drop on ball launch)
	tbhInit(lFly, 392, .1281/*0.3652*/, 3.03, 0.005152, 0, 43, 20); //initialize PID for left side of the flywheel //left side might be able to have a higher P
	tbhInit(rFly, 392, .1281/*0.3652*/, 3.03, 0.005152, 0, 43, 20); //initialize PID for right side of the flywheel //x.x481
	startTask(leftFwControlTask);
	startTask(rightFwControlTask);
}

//purple shooting (for skills)
void initializePIDMid() {
	//note the order of the parameters:
	//(controller, motor ticks per rev, KpNorm, KpBallLaunch, Ki, Kd, constant, RPM drop on ball launch)
	tbhInit(lFly, 392, 0.3291, 3.745, 0.005002, 0, 43, 27); //initialize PID for left side of the flywheel //left side might be able to have a higher P
	tbhInit(rFly, 392, 0.3291, 3.745, 0.005002, 0, 43, 27); //initialize PID for right side of the flywheel //x.x481
	startTask(leftFwControlTask);
	startTask(rightFwControlTask);
}

//purple shooting (for skills)
void initializePIDPurple() {
	//note the order of the parameters:
	//(controller, motor ticks per rev, KpNorm, KpBallLaunch, Ki, Kd, constant, RPM drop on ball launch)
	tbhInit(lFly, 392, 0.4281, 3.03, 0.005481, 0, 55, 20); //initialize PID for left side of the flywheel //left side might be able to have a higher P
	tbhInit(rFly, 392, 0.4281, 3.03, 0.005481, 0, 55, 20); //initialize PID for right side of the flywheel //x.x481
	startTask(leftFwControlTask);
	startTask(rightFwControlTask);
}

task stopFlywheel() {
	//in order to continuing monitoring RPM after the flywheel is stopped (to prevent balls from being shot when the flywheel is not spinning fast enough to shoot them out of the robot),
	//  the flywheel is stopped using a special instace of the PIC controller, which will monitor flywheel RPM until it reaches 5 and then shutdown the flywheel motors completely)
	//  This is structured such that starting the flywheel will immediately override anything this task does.
	while(1) {
		if(flywheelMode == 0.5) { //trigger this by changing the value of flywheelMode to 0.5 rather than using a function call
			//stop the flywheel tasks so we can restart them with our new controllers
			stopTask(leftFwControlTask);
			stopTask(rightFwControlTask);

			//create the new controllers.  Both P constants are high so that the motor value ends up being 0 (KpBallShot is the same as KpNorm so that the P constant has a constant value regardless of whether the controller thinks a ball has been shot [or if a ball has actually been shot])
			tbhInit(lFly, 392, 1, 1, 0, 0, 0, 20); //initialize PID for left side of the flywheel //left side might be able to have a higher P
			tbhInit(rFly, 392, 1, 1, 0, 0, 0, 20); //initialize PID for right side of the flywheel //x.x481

			//restart the flywheel tasks with these new controllers
			startTask(leftFwControlTask);
			startTask(rightFwControlTask);
			FwVelocitySet(lFly, 0, 0);
			FwVelocitySet(rFly, 0, 0);

			//wait for the flywheels to have a velocity <= 5 RPM (for this only one side needs to meet this condition since the sides are mechanically linked)
			while ((lFly.current > 5 || rFly.current > 5) && flywheelMode == 0.5) {
				//wait to continue
				wait1Msec(50);
			}

			//the above while loop can be exited for one of two reasons:
			//  1. flywheel velocity on one side drops below 5 RPM
			//  2. flywheel mode changes (i.e., the user selects a new flywheel mode [close, purple, or long]
			//In case 1, we can stop the flywheel completely.  In case 2, we need to stop the flywheel stop process and start the flywheel back up (starting the flywheel is
			//  handled in the usercontrol task).
			if (flywheelMode == 0.5) { //only shutdown the flywheel if the user hasn't restarted the flywheel
				//return to open-loop control so we can control the flywheel motor powers
				stopTask(leftFwControlTask);
				stopTask(rightFwControlTask);

				//turn off the flywheel motors
				setLeftFwSpeed(0);
				setRightFwSpeed(0);

				flywheelMode = 0; //make sure we know that the flywheel is fully stopped
			}
		}
		wait1Msec(50); //don't overload the cpu
	}
}

//stop flywheel (note: this function should only be used for autonomous code.  The stopFlywheel *task* handles flywheel stops during driver control
void stopFlywheelAuton() {
	//disable PIC control of the flywheels and switch to open-loop control
	stopTask(leftFwControlTask);
	stopTask(rightFwControlTask);
	//turn off the flywheel motors
	setLeftFwSpeed(0);
	setRightFwSpeed(0);

	flywheelMode = 0; //make sure we know that the flywheel is stopped
}

task drivetrainController() {
	int lYRequested,
	rYRequested,
	lYLastSent = 0,
	rYLastSent = 0,
	lY,
	rY,
	slewRateLimit = 15, //the maximum amount the drivetrain motor powers can increase or decrease in one cycle of the function
	threshold = 15; //the minimum positive or negative motor power to send a value other than 0 to the drivetrain motors
	bool slowMode = false; //when true, divides all requested drivetrain powers (lYRequested and rYRequested) by 3, then applies the slew rate, then sends motor powers
	while(true) {
		slowMode = (vexRT[Btn5D]) ? true : false;
		lYRequested = (slowMode) ? vexRT[Ch3]/3 : vexRT[Ch3]; //if slow mode is enabled, divide the requested motor power by 3
		rYRequested = (slowMode) ? vexRT[Ch2]/3 : vexRT[Ch2];
		if (abs(lYRequested - lYLastSent) > slewRateLimit) { //if the new power requested is greater than the slew rate limit
			if (lYRequested > lYLastSent) {
				lY += slewRateLimit; //only increase the power by the max allowed by the slew rate
				} else {
				lY -= slewRateLimit; //only decrease the power by the max allowed by the slew rate
			}
			} else {
			lY = (lYRequested == 0) ? 0 : lY;
		}
		lYLastSent = lY;
		if (abs(rYRequested - rYLastSent) > slewRateLimit) {
			if (rYRequested > rYLastSent) {
				rY += slewRateLimit;
				} else {
				rY -= slewRateLimit;
			}
			} else {
			rY = (rYRequested == 0) ? 0 : rY;
		}
		rYLastSent = rY;
		motor[lDriveFrontMid] = (abs(lY) > threshold) ? lY : 0;
		motor[lDriveBack] = (abs(lY) > threshold) ? lY : 0;
		motor[rDriveFront] = (abs(rY) > threshold) ? rY : 0;
		motor[rDriveMiddle] = (abs(rY) > threshold) ? rY : 0;
		motor[rDriveBack] = (abs(rY) > threshold) ? rY : 0;
		wait1Msec(15);
	}
}

void intakeChainDistance (int encoderCounts, int direction, float power, int time) {
	nMotorEncoder[intakeChain] = 0;
	time1[T2] = 0;
	while (abs(nMotorEncoder[intakeChain]) < encoderCounts && time1[T2] < time) {
		motor[intakeChain] = power*direction;
	}

	motor[intakeChain] = 0;
}

//run the intake chain for a distance, and keep the roller running while doing it
void intakeDistance (int encoderCounts, int direction, float power, int time) {
	nMotorEncoder[intakeChain] = 0;
	time1[T2] = 0;
	motor[intakeRoller] = 127*direction;
	while (abs(nMotorEncoder[intakeChain]) < encoderCounts && time1[T2] < time) {
		motor[intakeChain] = power*direction;
	}

	setIntakeMotors(0);
}

//needs to be revised
void longShotAuton(bool waitAtStart) {
	//if(waitAtStart) {
	//	wait1Msec(3000);
	//}
	//initializePIDLong();
	//FwVelocitySet(lFly,132.5,.7);
	//FwVelocitySet(rFly,132.5,.7);
	//wait1Msec(2000);
	//intakeDistance(150,1,125);
	//wait1Msec(2000);
	//intakeDistance(150,1,125);
	//wait1Msec(2000);
	//intakeDistance(300,1,125);
	//wait1Msec(2000);
	//intakeDistance(300,1,125);
	//wait1Msec(1500);
	//stopFlywheelAuton(); //use stopFlywheelAuton() function here since a task is used for flywheel stops during driver control
}

//needs to be revised
void closeShotAuton(bool waitAtStart) {
	if(waitAtStart) {
		wait1Msec(3000);
	}
	initializePIDShort();
	FwVelocitySet(lFly, 97.75, .5);
	FwVelocitySet(rFly, 97.75, .5);
	//driveDistance(3350, 1, 85);
	wait1Msec(500);
	//rotate(0,1);
	wait1Msec(250);
	setIntakeMotors(115); //turn on the intake to outtake the balls
	wait1Msec(1750); //wait long enough to shoot all the balls
	setIntakeMotors(0); //stop the intake
	stopFlywheelAuton(); //use stopFlywheelAuton() function here since a task is used for flywheel stops during driver control
}

//needs to be revised
void programmingSkills() {
	startTask(flashLED);
	initializePIDPurple();
	FwVelocitySet(lFly,115,.7);
	FwVelocitySet(rFly,115,.7);
	setIntakeMotors(125);
	wait1Msec(25000);
	stopFlywheelAuton(); //use stopFlywheelAuton() function here since a task is used for flywheel stops during driver control
	setIntakeMotors(0);
	//rotateDegrees(860,1);
	wait1Msec(750);
	//driveDistance(3375, -1, 85);
	setIntakeMotors(125);
	initializePIDPurple();
	FwVelocitySet(lFly,115,.7);
	FwVelocitySet(rFly,115,.7);
	wait1Msec(750);
	//rotateDegrees(895,-1);
	wait1Msec(500);
	setIntakeMotors(125);
	wait1Msec(25000);
}

task autonomous()
{
	if (pgmToRun == "R Side Long" || pgmToRun == "R Back Long"
		|| pgmToRun == "B Side Long"
	|| pgmToRun == "B Back Long") {
		longShotAuton(delayStart);
	} else if (pgmToRun == "B Side Close" || pgmToRun == "B Back Close"
		|| pgmToRun == "R Side Close"
	|| pgmToRun == "R Back Close") {
		closeShotAuton(delayStart);
		} else if (pgmToRun == "Prog. skills") {
		programmingSkills();
	}
}

bool userIntakeControl = true,
		 btn6UPressed = false,
		 indirectCloseShootStart = false; //when set to true, will start close shooting, once started, close shooting returns this variable to false
		 																	//   Important note: Use progStartCloseShooting() to change this variable - don't change the value of this variable directly.
		 																	//     	This variable will not be set to false until close shooting has been activated.
		 																	//      If using a joystick button to set this variable to true, make sure that this can only happen
		 																	//      when close shooting is NOT already running to avoid weird behavior and random close shooting activations
		 																	//      Example: If you set this to true when the flywheel is already is close shooting mode, the flywheel will
		 																	//         restart immediately after the shooting mode is changed or the flywheel is turned off.

/*
* A function to programmatically start close shooting if close shooting is currently not running.
* Uses the indirectCloseShootStart variable, but controls it to prevent unexpectedly close shooting activation.
* Use this function in lieu of directly changing the valueof indirectCloseShootStart
*/
void progStartCloseShooting() {
	if (flywheelMode != 1) { //only try to start close shooting if close shooting is not already running; otherwise, do nothing
		indirectCloseShootStart = true;
	}
}

/*
* Task that manages intake controls.  Decides what button 6U does depending on state of flywheel and balls in intake.
*/
task intakeController() {
	while (1) {
		if (userIntakeControl) {
					motor[intakeChain] = ((vexRT[Btn5U] - vexRT[Btn5D])*127);
					motor[intakeRoller] = ((vexRT[Btn6U] - vexRT[Btn6D])*127);
			}
		}
}

task liftController() {
	while(1) {
		if (vexRT[Btn8L] == 1 && vexRT[Btn8R] == 1) {
			//nMotorEncoder[lFlywheel] = 0
			//while (nMotorEncoder[lFlywheel] < 292)
		}

		if (vexRT[Btn8U] == 1 && vexRT[Btn8L] == 1) {
			//motor[fourBarRelease] = 127;
			} else {
			//motor[fourBarRelease] = 0;
		}

		wait1Msec(50);
	}
}

task countBallsInIntake() {
	int numConsecZeros = 0, //number of consecutive 0s return by the limit switch
			numConsecOnes = 0; //number of consecutive 1s return by the limit switch
	while(1) {

		if (ballsInIntake < 3 && rollerState == 1) { //if there are 2 or fewer balls in the intake, and intaking balls in
			while(!SensorValue[intakeLimit]) { //limit switch indicates that a ball is not at the bottom of the dangle
				wait1Msec(25); //wait until a ball approaches the dangle
			}
			while(numConsecZeros < 2) { //condition: SensorValue[intakeLimit]; wait until the ball has triggered the limit switch
				if (!SensorValue[intakeLimit]) {
					numConsecZeros++;
				} else {
					numConsecZeros = 0;
				}
				wait1Msec(25);
			}
		} else if (ballsInIntake <= 3 && rollerState == -1) { //if there are 3 or fewer balls in the intake and we are outtaking via the roller
			while(!SensorValue[intakeLimit]) { //limit switch indicates that a ball is not at the bottom of the dangle
				wait1Msec(25); //wait until a ball approaches the dangle
			}
			while(numConsecZeros < 2) { //condition: SensorValue[intakeLimit]; wait until the ball has triggered the limit switch
				if (!SensorValue[intakeLimit]) {
					numConsecZeros++;
				} else {
					numConsecZeros = 0;
				}
				wait1Msec(25);
			}
		} else if (ballsInIntake >= 3 && rollerState == 1) { //limit switch is pressed when there are 4 balls in the intake - do this when the third ball is in the intake, so ballsInIntake = 3
			while(numConsecOnes < 2) { //condition: !SensorValue[intakeLimit]; only do this when going forward - this means we are intaking
				if (SensorValue[intakeLimit]) {
					numConsecOnes++;
				} else {
					numConsecOnes = 0;
				}
				wait1Msec(25);
			}
		} else if (ballsInIntake > 3 && rollerState == -1) { //limit switch is pressed when there are 4 balls in the intake - do this when the fourth ball is in the intake, so ballsInIntake = 3
				while(SensorValue[intakeLimit]) { //only do this when going backward - this means we are releasing balls from the lower end of the intake; the 4th ball keeps the limt switch pressed
 					if (!SensorValue[intakeLimit]) {
						numConsecZeros++;
					} else {
						numConsecZeros = 0;
					}
					wait1Msec(25);
 				}
		}

		numConsecZeros = 0; //reset to zero once limit we move on
		numConsecOnes = 0;

		//NOTE: this doesn't account for balls leaving the intake via the flywheel
		//reach this point once the intake limit switch has been pressed and then released (so balls are counted after they are done passing the dangle)

		if (rollerState == 1) { //if the roller is moving forward
			ballsInIntake++; //increment the number of balls in the intake
			} else if (rollerState == -1) { //if the roller is moving backwards (indicated by a rollerState value of -1)
			ballsInIntake--; //decrement the number of balls in the intake
		}

		//limit the ballsinIntake variable to 0-4 (inclusive) only
		if (ballsInIntake > 4) {
			ballsInIntake = 4;
		} else if (ballsInIntake < 0) {
			ballsInIntake = 0;
		}

		if (flywheelMode != 3 || flywheelMode != 4) {
				outtakeOnly = false;
		} else {
				if (ballsInIntake == 4) {
				outtakeOnly = true;
			} else if (ballsInIntake < 4 && outtakeOnly) { //check if we can disable outttake only mode
				outtakeOnly = false;
			}
		}
		clearLCDLine(0);
		displayLCDPos(0,0);
		displayNextLCDNumber(ballsInIntake);

	}

}

task autoIntake() {
	while(1) {
		if (flywheelMode != 3 || flywheelMode != 4) { //don't auto-intake for purple or long shooting

			if(SensorValue[intakeLimit] && !vexRT[Btn5D] && ballsInIntake < 3 && rollerState == 1) { //if the intake limit switch is pressed
				userIntakeControl = false;
				intakeChainDistance(275,1,127,1500); //move the second stage up
				userIntakeControl = true;
				while(SensorValue[intakeLimit]) { //wait until the intake limit switch is no longer pressed so that the moveIntakeChain command doesn't run multiple times
					wait1Msec(75);
				}
			} else if (!overrideAutoIntake) { //only make userIntakeControl true if another automated task isn't running
				userIntakeControl = true;
			}

			wait1Msec(25);
		}
	}
}

void moveIntakeBack() {
	overrideAutoIntake = true;
	userIntakeControl = false;
	setIntakeRoller(0);
	intakeChainDistance(75,-1,127,1000);
	wait10Msec(20);
	userIntakeControl = true;
	overrideAutoIntake = false;
}

//needs intake line follower sensor
task intakeWatchDog() {
	//while(1) {
	//	if(flywheelMode > 0 && lFly.current < 30 && rFly.current < 30) {
	//		if(SensorValue[intakeBall] < 1500) { //sonar sensor values that indicate the presence of a ball
	//			outtakeOnly = true;
	//			moveIntakeBack(); //move the intake back so that the ball is not touching the flywheel
	//			wait1Msec(300);
	//		}
	//	} else {
	//			outtakeOnly = false;
	//	}
	//	wait1Msec(25);
	//}
}


task flywheelWatchdog() {
	while(1) {
		if (flywheelMode >= 1) { //if the flywheel is supposed to be running
			if (lFly.current == 0 || rFly.current == 0) { //if one side of the flywheel is not moving
				wait1Msec(275); //wait half a second to see if the flywheel just needs time to start
				if (lFly.current == 0 || rFly.current == 0) { //if the flywheel is still not moving
					flywheelMode = 0.5; //stop the flywheel (stopFlywheel task)
					yellowLEDFlashTime = 125;  //rapidly flash the yellow LED
				}
			}
		}
		if ((lFly.current > 0 || rFly.current > 0) && flywheelMode >= 1) { //if the flywheel is moving
			yellowLEDFlashTime = 0; //stop flashing the yellow LED since the flywheel is OK now
		}
		wait1Msec(25);
	}
}

task usercontrol()
{
	//initalize tasks to control various subsystems that need to run concurrently during driver control
	//some tasks below have not been tested yet and/or lack necessary hardware or sensors.  That's why they are commented out:
	// -intakeWatchdog: values need to be tuned
	// -liftController: actuation mechanisms not finished

	//tasks in use normally.  Comment out to test shooting
	startTask(intakeController);
	startTask(drivetrainController);
	//startTask(flashLED);
	//startTask(autoIntake);
	//startTask(countBallsInIntake);
	startTask(stopFlywheel);
	//startTask(flywheelWatchdog);

	//startTask(intakeWatchDog);
	//startTask(liftController);

	//initializePIDLong(); //prepare controller for long shooting
	////set long shooting velocities
	//FwVelocitySet(lFly,139,.7);
	//FwVelocitySet(rFly,139,.7);
	////yellowLEDFlashTime = 320;
	////overrideAutoIntake = true;
	//userIntakeControl = false;
	////wait1Msec(2300);
	//setIntakeMotors(127);


	while (true)
	{

		//if (vexRT[Btn8D]) { //reset (tare) the intake ball count
			//ballsInIntake = 0;
		//}

		//flywheel speed control
		//7U - long, 7L - purple, 7R - shoot from field edge, 5U - center 7D - short
		//8R - stop
		if (vexRT[Btn7U] == 1 && flywheelMode != 4) { //second condition prevents reinitialization of long shooting if the flywheel is currently in long shooting mode
				//mode 0.5 is for when the flywheel has been shutdown but is still spinning.  Since the control tasks are used for this process, the flywheel tasks need to be restarted.
				if (flywheelMode >= 0.5) { //if the flywheel is currently running (modes 0.5,1-4), we need to stop the controller tasks before re-initializing the PID controller
					stopTask(leftFwControlTask);
					stopTask(rightFwControlTask);
				}

				//ballsInIntake = 0; //reset the intake ball counter for simplicity

				//next 4 lines have to run every time to run flywheel
				flywheelMode = 4; //make sure we set the flywheel mode
				initializePIDLong(); //prepare controller for long shooting
				//set long shooting velocities
				FwVelocitySet(lFly,138,.7);
				FwVelocitySet(rFly,138,.7);

				//yellowLEDFlashTime = 320; //flash the yellow LED for pacing

		} else if (vexRT[Btn7R] == 1 && flywheelMode != 3.5) { //field edge shooting
				//mode 0.5 is for when the flywheel has been shutdown but is still spinning.  Since the control tasks are used for this process, the flywheel tasks need to be restarted.
				if (flywheelMode >= 0.5) { //if the flywheel is currently running (modes 0.5,1-4), we need to stop the controller tasks before re-initializing the PID controller
					stopTask(leftFwControlTask);
					stopTask(rightFwControlTask);
					userIntakeControl = true;
				}

				//ballsInIntake = 0; //reset the intake ball counter for simplicity

				//next 4 lines have to run every time to run flywheel
				flywheelMode = 3.5;

				//Uncomment these lines when this shooting mode has been tested
				//initializePIDFieldEdge();
				//FwVelocitySet(lFly,118.5,.7);
				//FwVelocitySet(rFly,118.5,.7);

		} else if (vexRT[Btn7L] == 1 && flywheelMode != 3) { //purple shooting
				//mode 0.5 is for when the flywheel has been shutdown but is still spinning.  Since the control tasks are used for this process, the flywheel tasks need to be restarted.
				if (flywheelMode >= 0.5) { //if the flywheel is currently running (modes 0.5,1-4), we need to stop the controller tasks before re-initializing the PID controller
					stopTask(leftFwControlTask);
					stopTask(rightFwControlTask);
					userIntakeControl = true;
				}

				//ballsInIntake = 0; //reset the intake ball counter for simplicity

				//next 4 lines have to run every time to run flywheel
				flywheelMode = 3;
				initializePIDMid();
				FwVelocitySet(lFly,114.85,.7);
				FwVelocitySet(rFly,114.85,.7);

		} /*else if (vexRT[Btn5U] == 1 && flywheelMode != 2) { //center shooting
				//mode 0.5 is for when the flywheel has been shutdown but is still spinning.  Since the control tasks are used for this process, the flywheel tasks need to be restarted.
				if (flywheelMode >= 0.5) { //if the flywheel is currently running (modes 0.5,1-4), we need to stop the controller tasks before re-initializing the PID controller
					stopTask(leftFwControlTask);
					stopTask(rightFwControlTask);
					userIntakeControl = true;
				}

				//ballsInIntake = 0; //reset the intake ball counter for simplicity

				//next 4 lines have to run every time to run flywheel


				//Uncomment these lines once midfield shooting has been tested
				flywheelMode = 3.5;
				initializePIDMid();
				FwVelocitySet(lFly,118.5,.7);
				FwVelocitySet(rFly,118.5,.7);

		}*/ else if ((vexRT[Btn7D] == 1) && flywheelMode != 1) { //close shooting
			//mode 0.5 is for when the flywheel has been shutdown but is still spinning.  Since the control tasks are used for this process, the flywheel tasks need to be restarted.
				if (flywheelMode >= 0.5) { //if the flywheel is currently running (modes 0.5,1-4), we need to stop the controller tasks before re-initializing the PID controller
					stopTask(leftFwControlTask);
					stopTask(rightFwControlTask);
					userIntakeControl = true;
				}

				//indirectCloseShootStart = false; //setting this to true will do the same thing as pressing Btn7D on the joystick.  Once the variable has activated

				//moveIntakeBack(); //move the intake back a little before startin the flywheel

				//ballsInIntake = 0; //reset the intake ball counter for simplicity

				//next 4 lines have to run every time to run flywheel
				flywheelMode = 1;
				initializePIDShort();
				FwVelocitySet(lFly, 94, .5);
				FwVelocitySet(rFly, 94, .5);

		} else if (vexRT[Btn8R] == 1 && flywheelMode >= 1) { //this is an else statement so that if two buttons are pressed, we won't switch back and forth between starting and stopping the flywheel
				//  flywheelMode needs to be >=1 and not >=0.5 because we don't want to stop the flywheel again if it is currently in the process of the stopping,
				//  although since the value of flywheelMode would not change in that case, it would appear as if nothing happened
				userIntakeControl = true; //make sure the driver can control the intake again
				//overrideAutoIntake = false; //allow the autoIntake task to have control over the userIntakeControl variable again

				//below line triggers flywheel shutdown procedure
				flywheelMode = 0.5;
		}


		//writeDebugStreamLine("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",nPgmTime,lFly.current, lFly.motor_drive, lFly.p, lFly.i, lFly.d, lFly.constant, 50*lFly.postBallLaunch, rFly.current, rFly.motor_drive, rFly.p, rFly.i, rFly.d, rFly.constant, 60*rFly.postBallLaunch);

		wait1Msec(25); //don't overload the CPU
	}
}
