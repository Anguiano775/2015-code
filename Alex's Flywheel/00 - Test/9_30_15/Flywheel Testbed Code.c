#pragma config(Sensor, dgtl1,  Encoder,        sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  Encoder2,       sensorQuadEncoder)
#pragma config(Motor,  port2,           Fly1,          tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port3,           Fly2,          tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           Fly3,          tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           Fly4,          tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           Fly5,          tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,          Fly6,          tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//


task main()
{
while(true)
{
    if(vexRT[Btn5U] == 1)
    {
      	motor[Fly1] = 100;
				motor[Fly2] = 100;
				motor[Fly3] = 100;
				motor[Fly4] = 100;
			  motor[Fly5] = 100;
			  motor[Fly6] = 100;
    }
    else if(vexRT[Btn5D] == 1)
    {
     	motor[Fly1] = -100;
			motor[Fly2] = -100;
			motor[Fly3] = -100;
			motor[Fly4] = -100;
			motor[Fly5] = -100;
			motor[Fly6] = -100;
    }

    else
    {
      	motor[Fly1] = 0;
				motor[Fly2] = 0;
				motor[Fly3] = 0;
				motor[Fly4] = 0;
				motor[Fly5] = 0;
				motor[Fly6] = 0;
    }
  }
}