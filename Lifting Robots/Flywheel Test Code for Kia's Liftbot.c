#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, dgtl12, led,            sensorLEDtoVCC)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Motor,  port6,           flyWheelL1,    tmotorVex393_MC29, openLoop, encoderPort, None)
#pragma config(Motor,  port7,           flyWheelL2,    tmotorVex393_MC29, openLoop, reversed, encoderPort, I2C_1)
#pragma config(Motor,  port8,           flyWheelR1,    tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           flyWheelR2,    tmotorVex393_MC29, openLoop, encoderPort, I2C_2)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//
task main()
{

while(1) {

}

}
