#pragma config(UART_Usage, UART1, uartVEXLCD, baudRate19200, IOPins, None, None)
#pragma config(UART_Usage, UART2, uartNotUsed, baudRate4800, IOPins, None, None)
#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in1,    powerExpander,  sensorNone)
#pragma config(Sensor, dgtl12, led,            sensorLEDtoVCC)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_3,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           rFlyTop,       tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           rFlyBottom,    tmotorVex393_MC29, openLoop, encoderPort, I2C_1)
#pragma config(Motor,  port4,           lFlyTop,       tmotorVex393_MC29, openLoop, encoderPort, None)
#pragma config(Motor,  port5,           lFlyBottom,    tmotorVex393_MC29, openLoop, reversed, encoderPort, I2C_2)
#pragma config(Motor,  port7,           intake,        tmotorVex393_MC29, openLoop, encoderPort, I2C_3)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

void driveFlywheel(float value) {
	motor[lFlyTop] = value;
	motor[lFlyBottom] = value;
	motor[rFlyTop] = value;
	motor[rFlyBottom] = value;
}
task rpm() {
	while(1){
		int startEncoder = nMotorEncoder[lFlyBottom];
		wait1Msec(25);
		int finalEncoder = nMotorEncoder[lFlyBottom];
		int deltaE = finalEncoder - startEncoder
		int RPM = deltaE/392*(1000/25)*60;

		writeDebugStreamLine("%f",RPM);
	}
}

task main()
{
startTask(rpm);

	int speed = 35;
	while (speed < 100) {
		driveFlywheel(speed + 5);
		wait1Msec(250);
		speed = speed + 5;
	}
	wait1Msec(30000);
	driveFlywheel(80);
	wait1Msec(120000);
	driveFlywheel(95);
	wait1Msec(300);
	driveFlywheel(80);
	wait1Msec(10000);
	driveFlywheel(60);
	wait1Msec(30000);
	driveFlywheel(55);
	wait1Msec(30000);
	driveFlywheel(50);
	wait1Msec(30000);
	driveFlywheel(40);
	wait1Msec(20000);
	speed = 36;
	while (speed > 24) {
		driveFlywheel(speed);
		wait1Msec(1250);
		speed = speed - 3;
	}
	driveFlywheel(0);

}
