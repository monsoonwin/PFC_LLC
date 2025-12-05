/****************************************************************************************
  * @file    Type3.c
  * @author  尖叫的变压器
  * @version V1.0.0
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#include "type3.h"

float      Vosc;                //三角波（模拟补偿器输出和三角波比较生成PWM）
float      Xn,Xn_1,Xn_2,Xn_3;   //差分方程输入项：输出电压-参考电压
float      Yn,Yn_1,Yn_2,Yn_3;   //差分方程输出项：误差放大器输出Verror
uint32_t   Pulse_with_type3p3z=100;
float      Vref_LLC;
/**********************仿真5us************************/
float k1=-0.415017720023f;            //差分方程系数：对应差分方程各项  OK的
float k2=0.37445855483f;
float k3=0.414919275447f;
float k4=-0.37455699940f;
float k5=1.113014372908f;
float k6=-0.039574719432f;
float k7=-0.073439653475f;

void type_3_int(void)
{

 Xn=0;         //误差X(n)
 Xn_1=0;       //误差X(n-1)
 Xn_2=0;       //误差X(n-1)
 Xn_3=0;       //误差X(n-2)
	
 Yn=0;         //补偿器输出Y(n)
 Yn_1=0;       //补偿器输出Y(n-1)
 Yn_2=0;       //补偿器输出Y(n-2)
 Yn_3=0;       //补偿器输出Y(n-3)
 Vosc  =3.3f;
 Pulse_with_type3p3z=Min_period;
 Vref_LLC=0.0;
}
//计算LLC环路，可以参考哔哩哔哩通用视频教程
void type_3_cal(float V)//TYPE3算法
{
	 Xn=V-Vref_LLC;          //输出电压-参考电压
	 Yn=k1*Xn+k2*Xn_1+k3*Xn_2+k4*Xn_3+k5*Yn_1+k6*Yn_2+k7*Yn_3; //这个方程代表模拟电路type3补偿器
	 if(Yn>=Vosc)Yn=Vosc;		 //限幅                   
	 else if(Yn<=0)Yn=0;     //限幅
	 
	 Xn_3=Xn_2;Xn_2=Xn_1;Xn_1=Xn;//移位				 														 
	 Yn_3=Yn_2;Yn_2=Yn_1;Yn_1=Yn;	
	 
	Pulse_with_type3p3z=Min_period+Yn*6981.0f;
	 
	if(Pulse_with_type3p3z>Max_period)Pulse_with_type3p3z=Max_period;
	else if(Pulse_with_type3p3z<Min_period)Pulse_with_type3p3z=Min_period;

	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].PERxR=Pulse_with_type3p3z;
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR=Pulse_with_type3p3z>>1;//更新占空比
}

//计算LLC环路，可以参考哔哩哔哩通用视频教程
void type_3_tustin(void)
{
	double R1,R2,R3,C1,C2,C3,Ts;
	double a1,a2,a3,a4,a5,k0,b1,b2,b3,b4,b5;
	double Gain_adj;

	//参数获取，仿真+实际调试
	R1=12.6*Kom;C1=10*pF;
	R2=1.0*Kom;C2=100*nF;
	R3=10.0*om;C3=3*nF;

  Ts=15.0*uS ;//%sampling period
	
  Gain_adj=50.0f;
	
	a1=R1*C1/Ts;
	a2=R1*C2/Ts;
	a3=R1*C3/Ts;
	a4=R2*C2/Ts;
  a5=R3*C3/Ts;
	
	b1=(a3*a4+a4*a5)*4;
	b2=(a4+a3+a5)*2;
	b3=(a1*a4+a1*a5+a2*a5)*4;
	b4=(a1*a4*a5)*8;
	b5=(a1+a2)*2;
	
	k0=(b5+b3+b4);
	k1=(-1-b2-b1)/k0;
	k2=(-3-b2+b1)/k0;
	k3=(-3+b2+b1)/k0;
	k4=(-1+b2-b1)/k0;
	k5=-(b5-b3-3*b4)/k0;
	k6=-(-b5-b3+3*b4)/k0;
	k7=-(-b5+b3-b4)/k0;
	
	k1=k1/Gain_adj;
  k2=k2/Gain_adj;
  k3=k3/Gain_adj;
  k4=k4/Gain_adj;
}


