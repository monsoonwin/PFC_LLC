/****************************************************************************************
  * @file    PID.c
  * @author  尖叫的变压器
  * @version V1.0.0
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#include "PID.h"
/****************************电压环参数**********************************************/
type_pid V_PI={0};             //电压环PI
type_pid I_PI={0};             //电流环PI
uint16_t Pulse_width=0.0f;     //占空比
float    PWM_K=0.0f;           //脉宽常数
float    Vref_PFC;             //电压参考值
float    Iref_PFC;             //电流参考值
float    PFC_duty=0.0f;        //PFC占空比
float    PFC_duty_max=0.0f;    //PFC最大占空比
float    PFC_duty_max_start=0.0;

void PID_INT(void)//PID初始化
{
	V_PI.error1=0.00f;
	V_PI.error2=0.00f;
	V_PI.error_add=0.0f;
	V_PI.Ki=0.007f;
	V_PI.Kp=0.007f;
	V_PI.Kp_i=V_PI.Ki+V_PI.Kp;
	
	I_PI.error1=0.0f;
	I_PI.error2=0.0f;
	I_PI.error_add=0.0f;
	I_PI.Ki=0.15f;
	I_PI.Kp=0.6f;
	I_PI.Kp_i=I_PI.Ki+I_PI.Kp;
	Pulse_width=100.0f;
	PWM_K=PWM_PERIOD*0.303030f; //脉宽常数
	Vref_PFC=0.0f;
	Iref_PFC=0.0f;
	PFC_duty=0.0f;
	PFC_duty_max=0.0f;
	PFC_duty_max_start=0.0f;
}
extern float   AC_peak_voltage;  //AC交流输入电压瞬时值
extern float   AC_RMS;           //AC交流输入有效值
//电压外环+电流内环，可以到哔哩哔哩观看我们的通用视频教程，千变万变，PI控制不会变
void PID_loop(float voltage,float Current)
{
	//电压环
	V_PI.error1=(Vref_PFC-voltage)*0.01f;//获取误差差值
	V_PI.error_add+=V_PI.Kp_i*V_PI.error1-V_PI.Kp*V_PI.error2;//PI增量
	V_PI.error2=V_PI.error1;//移位
	if(voltage>410)V_PI.error_add-=0.0002f;
	if(V_PI.error_add>3.0f)V_PI.error_add=3.0f;//限幅
	else if(V_PI.error_add<0.001f)V_PI.error_add=0.001f;////限幅
	//电流环
	Iref_PFC=AC_peak_voltage*V_PI.error_add;//电压外环输出作为电流内环参考，让参考和电网电压相位一致，乘法
	/*
	限幅，防止电流参考无限大，比如让内环电流参考值为几十安培，显然不合理
	1)V_PI.error_add 最大=3.0
	2)电阻分压系数10K/(680K+680K+10K)=1/137
	3)AC_RMS=264V时AC_peak_voltage最大=264*1.414/137=2.7226，Iref_PFC最大=2.7226*3=8.1678，
	4)AC_RMS=90V 时AC_peak_voltage最大=90*1.414/137=0.92890，Iref_PFC最大=0.92890*3=2.7867
	5)我们期望PFC峰值电流最大1.414A，那么AC_RMS=264V衰减系数=1.414/8.1678=0.1731188
	6)我们期望PFC峰值电流最大1.414A，那么AC_RMS=90V衰减系数=1.414/2.7867=0.507410
	7)(90,0.507410),(264,0.1731188),两点确定一条直线y=kx+b,k=-0.001921213793,b=0.68031924
	8)功率将严格限制在：90瓦@AC90V，220瓦@220V,264瓦@264V，考虑到电感饱和等，实际会更小
	*/
	Iref_PFC=(0.68031924f-0.001921213793f*AC_RMS)*Iref_PFC; 
	I_PI.error1=Iref_PFC-Current;
	I_PI.error_add+=I_PI.Kp_i*I_PI.error1-I_PI.Kp*I_PI.error2;
	I_PI.error2=I_PI.error1;
	if(I_PI.error_add>3.0f)I_PI.error_add=3.0f;//限幅
	else if(I_PI.error_add<0.01f)I_PI.error_add=0.01f;//限幅	
  
	//计算占空比
	PFC_duty=I_PI.error_add*0.333333f;
	PFC_duty_max=1-0.33827f*AC_peak_voltage;//Dmax=1-Vin/Vomax,Vomax=405V.
	if(PFC_duty>PFC_duty_max)PFC_duty=PFC_duty_max;
	if(PFC_duty>PFC_duty_max_start)PFC_duty=PFC_duty_max_start;
	Pulse_width=PWM_PERIOD*PFC_duty;
	if(Pulse_width<40)Pulse_width=40;
	//更新寄存器
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP1xR=Pulse_width;//更新占空比
}

