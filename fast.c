#include "HardwareInfo.c"
#include <SetServoBoard.h>
#include <SetServo.h>
#include <SetOneServo.h>
#include <SetMotorFreq.h>
#include "JMLib.c"
#include <SetMotor_Code.h>
#include <SetData.h>
#include <GetSysTimeMs.h>
#include <SetSysTime.h>
#include <GetTraceV2I2C.h>
#include <GetTouchScreenX.h>
#include <GetButton1.h>
#include <SetNS.h>
#include <SetTenthS.h>
#include <SetInBeep.h>
#include <SetLCD3Char.h>


//常量
const int c_i_Kp = 15,c_i_Kd = 75;
//变量
int g_i_SensorState[7], g_i_LastReturn;

//函数
void MotorRun(int mr_i_leftSpeed, int mr_i_rightSpeed);
void SensorCal();
int GetSensorState();
void LineFollow_Sensor(int lfs_i_sensor, int lfs_i_speed);
void LineFollow_Time(long lft_l_time, int lft_i_speed);
void Action_Sensor(int as_i_sensor1, int as_i_sensor2, int as_i_leftSpeed, int as_i_rightSpeed);
void Motor(int m_i_leftSpeed, int m_i_rightSpeed,int m_i_time);
//SetOneServo( _SERVOCTRL_:which ,unsigned char:channel ,unsigned char:angle ,unsigned int:time );
//SetServo( _SERVO_:which ,unsigned char:angle );

//************************动作函数*****************************//

void loop(){
	LineFollow_Sensor(5, 20);
	SetServo( _SERVO_1_ ,45);
}










//************************************************************//
int main(void){
	int m_i_loopi;
	int m_i_tmp;
	
	X3RCU_Init();
	
	while(GetTouchScreenX() <= 10)
		if(GetButton1()){
			SetNS(1);
			SensorCal();
		}
	loop();
	MotorRun(0, 0);			
	return 0;
}


void MotorRun(int mr_i_leftSpeed, int mr_i_rightSpeed){
	if(mr_i_leftSpeed > 0){
		SetMotor_Code(1, 0, mr_i_leftSpeed);
	}else if(mr_i_leftSpeed == 0){
		SetMotor_Code(1, 1, 100);
	}else{
		SetMotor_Code(1, 2, mr_i_leftSpeed * -1);
	}
		
	if(mr_i_rightSpeed > 0){
		SetMotor_Code(2, 0, mr_i_rightSpeed);
	}else if(mr_i_rightSpeed == 0){
		SetMotor_Code(2, 1, 100);
	}else{
		SetMotor_Code(2, 2, mr_i_rightSpeed * -1);
	}
}

void SensorCal(){
	int sc_i_loopi, sc_i_loopj;
	
	SetSysTime();	
	
	for(sc_i_loopi = 1; sc_i_loopi <= 6; sc_i_loopi++){
		while(GetSysTimeMs() < 500 * sc_i_loopi){
			if(sc_i_loopi % 2 == 1){
				MotorRun(15, 15);
			}else{
				MotorRun(-15, -15);
			}
		}
		for(sc_i_loopj = 0; sc_i_loopj <= 5000; sc_i_loopj++)
			MotorRun(0, 0);
	}
}

int GetSensorState(){
	int gss_i_sensorRead;
	int gss_i_loopi;
	int gss_i_sensorCount = 0;
	
	gss_i_sensorRead = GetTraceV2I2C(_TRACEV2_TraceSensor_, 9);
	
	for(gss_i_loopi = 0; gss_i_loopi <= 6; gss_i_loopi++){
		g_i_SensorState[gss_i_loopi] = gss_i_sensorRead % 2;
		if(g_i_SensorState[gss_i_loopi] == 1){
			gss_i_sensorCount++;
		}
		gss_i_sensorRead = gss_i_sensorRead / 2;
	}
		
	if(gss_i_sensorCount == 1){
		for(gss_i_loopi = 0; gss_i_loopi <= 6; gss_i_loopi++)
			if(g_i_SensorState[gss_i_loopi] == 1){
				g_i_LastReturn = gss_i_loopi * 2 - 6;
				break;
			}
	}else if(gss_i_sensorCount == 2){
		for(gss_i_loopi = 0; gss_i_loopi <= 6; gss_i_loopi++)
			if(g_i_SensorState[gss_i_loopi] == 1){
				g_i_LastReturn = gss_i_loopi * 2 - 5;
				break;
			}
	}else if(gss_i_sensorCount == 3){
		for(gss_i_loopi = 0; gss_i_loopi <= 6; gss_i_loopi++)
			if(g_i_SensorState[gss_i_loopi] == 1){
				g_i_LastReturn = gss_i_loopi * 2 - 4;
				break;
			}
	}else if(gss_i_sensorCount == 0){
		if(g_i_LastReturn == 4){
			g_i_LastReturn = 5;
		}else if(g_i_LastReturn == -4){
			g_i_LastReturn = -5;
		}else if(g_i_LastReturn == 6){
			g_i_LastReturn = 7;
		}else if(g_i_LastReturn == -6){
			g_i_LastReturn = -7;
		}
	}
	return g_i_LastReturn;
}

void LineFollow_Sensor(int lfs_i_sensor, int lfs_i_speed){
	lfs_i_sensor--;
	int lfs_i_error;
	int lfs_i_leftSpeed, lfs_i_rightSpeed;
	int lfs_i_leftLastSpeed, lfs_i_rightLastSpeed;
	int lfs_i_leftRunSpeed, lfs_i_rightRunSpeed;
	int lfs_i_breakCount = 0;
	
	while(1){
		lfs_i_error = GetSensorState();
		if(g_i_SensorState[lfs_i_sensor]){
			lfs_i_breakCount++;
			if(lfs_i_breakCount >= 3)
				break;
		}else{
			lfs_i_breakCount = 0;
			lfs_i_leftSpeed = lfs_i_speed + lfs_i_speed * c_i_Kp / 100 * lfs_i_error;
			lfs_i_rightSpeed = lfs_i_speed - lfs_i_speed * c_i_Kp / 100 * lfs_i_error;
			if(lfs_i_leftLastSpeed != 0 && lfs_i_rightLastSpeed != 0){
				lfs_i_leftRunSpeed = lfs_i_leftSpeed + (lfs_i_leftSpeed - lfs_i_leftLastSpeed) * c_i_Kd / 100;
				lfs_i_rightRunSpeed = lfs_i_rightSpeed + (lfs_i_rightSpeed - lfs_i_rightLastSpeed)  * c_i_Kd / 100;
			}
			lfs_i_leftLastSpeed = lfs_i_leftSpeed;
			lfs_i_rightLastSpeed = lfs_i_rightSpeed;
			MotorRun(lfs_i_leftRunSpeed, lfs_i_rightRunSpeed);
		}
	}
	while(1){
		lfs_i_error = GetSensorState();
		if(g_i_SensorState[lfs_i_sensor] == 0){
			lfs_i_breakCount++;
			if(lfs_i_breakCount >= 3)
				break;
		}else{
			lfs_i_breakCount = 0;
			lfs_i_leftSpeed = lfs_i_speed + lfs_i_speed * c_i_Kp / 100 * lfs_i_error;
			lfs_i_rightSpeed = lfs_i_speed - lfs_i_speed * c_i_Kp / 100 * lfs_i_error;
			if(lfs_i_leftLastSpeed != 0 && lfs_i_rightLastSpeed != 0){
				lfs_i_leftRunSpeed = lfs_i_leftSpeed + (lfs_i_leftSpeed - lfs_i_leftLastSpeed) * c_i_Kd / 100;
				lfs_i_rightRunSpeed = lfs_i_rightSpeed + (lfs_i_rightSpeed - lfs_i_rightLastSpeed)  * c_i_Kd / 100;
			}
			lfs_i_leftLastSpeed = lfs_i_leftSpeed;
			lfs_i_rightLastSpeed = lfs_i_rightSpeed;
			MotorRun(lfs_i_leftRunSpeed, lfs_i_rightRunSpeed);
		}
	}
}

void LineFollow_Time(long lft_l_time, int lft_i_speed){
	int lft_i_error;
	int lft_i_leftSpeed, lft_i_rightSpeed;
	int lft_i_leftLastSpeed, lft_i_rightLastSpeed;
	int lft_i_leftRunSpeed, lft_i_rightRunSpeed;
	
	SetSysTime();	
	
	while(GetSysTimeMs() < lft_l_time){
		lft_i_error = GetSensorState();
		lft_i_leftSpeed = lft_i_speed + lft_i_speed * c_i_Kp / 100 * lft_i_error;
		lft_i_rightSpeed = lft_i_speed - lft_i_speed * c_i_Kp / 100 * lft_i_error;
		if(lft_i_leftLastSpeed != 0 && lft_i_rightLastSpeed != 0){
			lft_i_leftRunSpeed = lft_i_leftSpeed + (lft_i_leftSpeed - lft_i_leftLastSpeed) * c_i_Kd / 100;
			lft_i_rightRunSpeed = lft_i_rightSpeed + (lft_i_rightSpeed - lft_i_rightLastSpeed)  * c_i_Kd / 100;
		}
		lft_i_leftLastSpeed = lft_i_leftSpeed;
		lft_i_rightLastSpeed = lft_i_rightSpeed;
		MotorRun(lft_i_leftRunSpeed, lft_i_rightRunSpeed);
	}
}

void Action_Sensor(int as_i_sensor1, int as_i_sensor2, int as_i_leftSpeed, int as_i_rightSpeed){
	int as_i_breakCount = 0;
	
	while(1){
		GetSensorState();
		if(g_i_SensorState[as_i_sensor1] == 0){
			as_i_breakCount++;
			if(as_i_breakCount >= 3)
				break;
		}else{
			as_i_breakCount = 0;
			MotorRun(as_i_leftSpeed, as_i_rightSpeed);
		}
	}
	while(1){
		GetSensorState();
		if(g_i_SensorState[as_i_sensor1]){
			as_i_breakCount++;
			if(as_i_breakCount >= 3)
				break;
		}else{
			as_i_breakCount = 0;
			MotorRun(as_i_leftSpeed, as_i_rightSpeed);
		}
	}
	
	while(1){
		GetSensorState();
		if(g_i_SensorState[as_i_sensor2] == 0){
			as_i_breakCount++;
			if(as_i_breakCount >= 3)
				break;
		}else{
			as_i_breakCount = 0;
			MotorRun(as_i_leftSpeed, as_i_rightSpeed);
		}
	}	
	while(1){
		GetSensorState();
		if(g_i_SensorState[as_i_sensor2]){
			as_i_breakCount++;
			if(as_i_breakCount >= 3)
				break;
		}else{
			as_i_breakCount = 0;
			MotorRun(as_i_leftSpeed / 2, as_i_rightSpeed / 2);
		}
	}
}
void Motor(int m_i_leftSpeed, int m_i_rightSpeed,int m_i_time){
	MotorRun(m_i_leftSpeed, m_i_rightSpeed);
	SetTenthS(m_i_time);
	MotorRun(0, 0);
}

