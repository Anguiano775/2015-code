#pragma config(Motor,  port1,            ,             tmotorVex393HighSpeed_HBridge, openLoop)
#pragma config(Motor,  port2,            ,             tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port3,            ,             tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port4,            ,             tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port5,            ,             tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port6,            ,             tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port7,            ,             tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port8,            ,             tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port9,            ,             tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port10,           ,             tmotorVex393HighSpeed_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

task main()
{
	while(1) {
		motor[port1] = 127;
		motor[port2] = 127;
		motor[port3] = 127;
		motor[port4] = 127;
		motor[port5] = 127;
		motor[port6] = 127;
		motor[port7] = 127;
		motor[port8] = 127;
		motor[port9] = 127;
		motor[port10] = 127;
	}
}
