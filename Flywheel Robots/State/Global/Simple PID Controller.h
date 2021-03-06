#ifndef SimplePIDController_h
#endif
#define SimplePIDController_h
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//
//The basis for this code is from James Pearman (see below).  However, we have
//modified the structure a bit to suit our needs and namely, to accomdodate our
//flywheel, which has 2 independently moving sides.
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2015                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     flywheel.c                                                   */
/*    Author:     James Pearman                                                */
/*    Created:    28 June 2015                                                 */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00  28 June 2015 - Initial release                        */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    The author is supplying this software for use with the VEX cortex        */
/*    control system. This file can be freely distributed and teams are        */
/*    authorized to freely use this program , however, it is requested that    */
/*    improvements or additions be shared with the Vex community via the vex   */
/*    forum.  Please acknowledge the work of the authors when appropriate.     */
/*    Thanks.                                                                  */
/*                                                                             */
/*    Licensed under the Apache License, Version 2.0 (the "License");          */
/*    you may not use this file except in compliance with the License.         */
/*    You may obtain a copy of the License at                                  */
/*                                                                             */
/*      http://www.apache.org/licenses/LICENSE-2.0                             */
/*                                                                             */
/*    Unless required by applicable law or agreed to in writing, software      */
/*    distributed under the License is distributed on an "AS IS" BASIS,        */
/*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. */
/*    See the License for the specific language governing permissions and      */
/*    limitations under the License.                                           */
/*                                                                             */
/*    The author can be contacted on the vex forums as jpearman                */
/*    or electronic mail using jbpearman_at_mac_dot_com                        */
/*    Mentor for team 8888 RoboLancers, Pasadena CA.                           */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    An example of flywheel/shooter velocity control using the TBH algorithm  */
/*    Test system uses three motors with 25:2 gearing to the flywheel.         */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

// Update inteval (in mS) for the flywheel control loop
#define FW_LOOP_SPEED              25

// Maximum power we want to send to the flywheel motors
#define FW_MAX_POWER              127

// Structure to gather all the flywheel ralated data
typedef struct _fw_controller {
    long            counter;                ///< loop counter used for debug

    // encoder tick per revolution
    float           ticks_per_rev;          ///< encoder ticks per revolution

    // Encoder
    long            e_current;              ///< current encoder count
    long            e_last;                 ///< last encoder count
    long						encoder_timestamp;      ///< time of last encoder reading; helps prevent issues relating to I2C timing with multiple IEMs
		long						encoder_timestamp_last; ///< time of second to last encoder reading

    // velocity measurement
    float           v_current;              ///< current velocity in rpm
    float						v_last;									///< last velocity in rpm
    //long            v_time;                 ///< Time of last velocity calculation

    // PID control algorithm variables
    long            target;                 ///< target velocity
    long            current;                ///< current velocity
    long            last;                   ///< last velocity
    float           error;                  ///< error between actual and target velocities
    float           last_error;             ///< error last time update called
    float						last_rpm_average;				///< RPM value used in calculations during the last update
    float						older_125ms_rpm; 				///< RPM value from 125 ms, used for ball shot detection
    float						errorSum;
    float           gain;                   ///< gain
    float						KpNorm;									///< P constant used during normal flywheel operation (idle, essentially)
    float						KpBallLaunch;						///< P constant used to speed up recovery after a ball is launched by the flywheel
    float						ballLaunchVelocityDrop;  ///< how much the flywheel velocity drops after a ball is launched (a total over the last 5 cycles)
    float						Ki;
    float						Kd;
    float						constant;								///< constant in PID equation
    float						p;
    float						i;
    float						d;
    float           drive;                  ///< final drive out of TBH (0.0 to 1.0)
    float           drive_at_zero;          ///< drive at last zero crossing
    long            first_cross;            ///< flag indicating first zero crossing
    float           drive_approx;           ///< estimated open loop drive
    float						MOTOR_TPR;							///< ticks per rev for IME

    //for exponential weighted moving average
    float						rpm_average;						///< the current RPM value that TBH should use
    float						raw_last_rpm;						///< only for debugging the weighted average
    float						alpha;									///< constant for average calculation

    bool						postBallLaunch;					///< keeps track of whether a ball has just been launched
    float						last_5_rpm[5];

    // final motor drive
    long            motor_drive;            ///< final motor control value
    } fw_controller;

void tbhInit (fw_controller *fw, float MOTOR_TPR, float KpNorm, float KpBallLaunch, float Ki, float Kd, float constant, float ballLaunchVelocityDrop) {
	fw->MOTOR_TPR = MOTOR_TPR;
	fw->ticks_per_rev = MOTOR_TPR;
	fw->KpNorm = KpNorm;
	fw->KpBallLaunch = KpBallLaunch;
	fw->Ki = Ki;
	fw->Kd = Kd;
	fw->constant = constant;
	fw->alpha = 1;
	fw->postBallLaunch = false;
	fw->ballLaunchVelocityDrop = ballLaunchVelocityDrop;
	//ensure that the variables that store previous values start at 0 (i.e., will have a value and not be null/empty)
	fw->encoder_timestamp_last = 0;
	fw->e_last = 0;
	fw->last_error = 0;
	fw->last_rpm_average = 0;
	fw->older_125ms_rpm = 0;
	fw->v_last = 0;
	fw->errorSum = 0;
}

void getNewAverage(fw_controller *fw, float newVal) {
	if(!fw->rpm_average) { //if there's no average yet, just return the value given
		fw->rpm_average = newVal;
	} else {
	fw->rpm_average = fw->rpm_average*(1 - fw->alpha) + (newVal*fw->alpha);
	}
	fw->raw_last_rpm = newVal;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Set the controller position                                    */
/** @param[in]  fw pointer to flywheel controller structure                    */
/** @param[in]  desired velocity                                               */
/** @param[in]  predicted_drive estimated open loop motor drive                */
/*-----------------------------------------------------------------------------*/
void
FwVelocitySet( fw_controller *fw, int velocity, float predicted_drive )
{
    // set target velocity (motor rpm)
    fw->target        = velocity;

    // Set error so zero crossing is correctly detected
    fw->error         = fw->target - fw->rpm_average;
    fw->last_error    = fw->error;

    // Set predicted open loop drive value
    fw->drive_approx  = predicted_drive;
    // Set flag to detect first zero crossing
    fw->first_cross   = 1;
    // clear tbh variable
    fw->drive_at_zero = 0;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Calculate the current flywheel motor velocity                  */
/** @param[in]  fw pointer to flywheel controller structure                    */
/*-----------------------------------------------------------------------------*/
void
FwCalculateSpeed( fw_controller *fw )
{
    int     delta_ms;
    int     delta_enc;

    //The current encoder value is set by the control task and saved in the fw->e_current variable

    // This is just used so we don't need to know how often we are called
    // how many mS since we were last here
    delta_ms   = fw->encoder_timestamp - fw->encoder_timestamp_last;
    fw->encoder_timestamp_last = fw->encoder_timestamp;

    // Change in encoder count
    delta_enc = (fw->e_current - fw->e_last);

    // save last position
    fw->e_last = fw->e_current;

    // Calculate velocity in rpm
    if (delta_ms > 0) { //prevent divide by 0 errors
    	fw->v_current = (1000.0 / delta_ms) * delta_enc * 60.0 / fw->ticks_per_rev;

		//only calculate a new average if there was a change in time
  	getNewAverage(fw, fw->v_current); //this will take the latest calculated RPM average
    																	//value and factor it into the average, which is then
   																		//the RPM value used in the TBH calculation
   }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Update the velocity tbh controller variables                   */
/** @param[in]  fw pointer to flywheel controller structure                    */
/*-----------------------------------------------------------------------------*/
void
FwControlUpdateVelocityTbh( fw_controller *fw )
{
    // calculate error in velocity
    // target is desired velocity
		// rpm_average is a weighted exponential moving average and is the RPM used for calculations
		// error is positive if we're below the target and negative if we are above.
		fw->error = fw->target - fw->rpm_average; //current error for P and D terms
		//fw->rpm_average - fw->last_rpm_average; //positive if RPM is increasing, 0 if no change, and negative if decreasing

		if (fw->older_125ms_rpm != 0 && fw->older_125ms_rpm - fw->rpm_average >= fw->ballLaunchVelocityDrop) { //if the error from 5 cycles (125 ms) ago is greater than or equal to the velocity drop indicating a ball launch, switch to ball launch mode
			fw->postBallLaunch = true;
		} else if (fw->error <= 5 && fw->error >= -5) {
			fw->postBallLaunch = false;
		}

		//only add to the integral sum if this cycle is not part of the post-ball launch period
		if (!fw->postBallLaunch) { //only add to the integral when the flywheel has recovered from shooting a ball, because that error is caused by balls (artificial disturbances to the system)
			fw->errorSum += fw->error; //add error to the sum of past errors for I term
		}

		//use error values previously calculated and constants to calculate P, I, and D terms

		//after a ball is shot, the P constant causes the flywheel velocity to overshoot the target
		//To remedy this, the P constant should be lower normally and then increase after the large velocity drop
		//caused by a ball being launched.  The Kpnorm constant is for times when the flywheel is just running.  The Kphigh constant
		//is for times when the ball is launched, from the initial velocity drop to the return to normal velocity.
		//Kp high will be used when fw->postBallLaunch is set to true.  fw->postBallLaunch will be set to true as soon as the velocity drop is detected
		//(error > ballLaunchVelocityDrop)
		//and reset to false from the velocity gets near the setpoint (error < 5).
		fw->p = (fw->postBallLaunch) ? fw->error * fw->KpBallLaunch : fw->error * fw->KpNorm;
		fw->i = fw->errorSum * fw->Ki;
		fw->d = (fw->error - fw->last_error) * fw->Kd;
    fw->drive = fw->p + fw->i + fw->d + fw->constant;

    // Clip - we are only going forwards
    if( fw->drive > 127 )
          fw->drive = 127;
    if( fw->drive < 0 )
          fw->drive = 0;

    /*// Check for zero crossing
    if( sgn(fw->error) != sgn(fw->last_error) ) {
        // First zero crossing after a new set velocity command
        if( fw->first_cross ) {
            // Set drive to the open loop approximation
            fw->drive = fw->drive_approx;
            fw->first_cross = 0;
        }
        else
            fw->drive = 0.5 * ( fw->drive + fw->drive_at_zero );

        // Save this drive value in the "tbh" variable
        fw->drive_at_zero = fw->drive;
    }*/

    //move back the last 5 RPM values to get a new value for older_125ms_rpm
    for (int i = 0; i < 4; i++) {
    	fw->last_5_rpm[i] = fw->last_5_rpm[i + 1];
  	}

  	fw->older_125ms_rpm = fw->last_5_rpm[0]; //save the RPM from 125ms ago
  	fw->last_5_rpm[4] = fw->rpm_average;
  	fw->last_error = fw->error; //save last error
    fw->last_rpm_average = fw->rpm_average; //save last RPM used in calculations
}
