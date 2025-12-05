/****************************************************************************************
  * @file    state_machine.c
  * @author  尖叫的变压器
  * @version V1.0.0
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#include "state_machine.h"
/**********************定义变量**********************************/
float   PFC_voltage     =0.0f;  //PFC升压390V
float   PFC_current     =0.0f;  //PFC电感电流
float   AC_peak_voltage =0.0f;  //AC交流输入电压瞬时值
float   AC_RMS2         =0.0f;  //AC交流输入有效值
float   AC_RMS          =0.0f;  //AC交流输入有效值

float   Vout            =0.0f;  //LLC输出电压
float   Iout            =0.0f;  //LLC输出电流

float   PFC_ovp         =0.0f;  //输入过压保护点
float   LLC_ovp         =0.0f;  //输出过压保护点
float   LLC_ocp         =0.0f;  //输出电流过流保护点
float   AC_OVP          =0.0f;  //交流电压过压保护点
float   AC_UVP          =0.0f;  //交流电压欠压保护点
float   AC_OCP          =0.0f;  //交流电压过流保护点

extern float      Vref_LLC;     //LLC输出电压参考值
extern float      Vref_PFC;     //PFC输出电压参考值
extern float      PFC_duty_max_start;//PFC启动时，限制最大占空比，防止电感饱和

uint32_t OCP_CNT1       =0;     //OCP过流次数计数器
uint32_t OVP_CNT        =0;     //OVP过压次数计数器
uint16_t ACRMS_CNT      =0;     //记录采集AC电压点数
uint16_t ACRMS_CNT1     =0;

/**********************定义标志位*********************************/
DP_FlagStatus    flag_Soft_start =Run;
DP_FlagStatus    flag_start_cnt  =Run;
DP_FlagStatus    ERROR_flag      =STOP;
DP_FlagStatus    Data_update_flag=STOP;
Error_FlagStatus flag_Vin_ovp    =Normal;
Error_FlagStatus flag_Vin_uvp    =Normal;
/**********************外部变量**************************************/   
extern   uint16_t  ADC1_RESULT[3];
extern   uint8_t   PC_command[50];
/********************************************************************/
//外部定义结构体：
extern DMA_HandleTypeDef    ADC1_DMA_Handler;
extern HRTIM_HandleTypeDef  HRTIM1_structure;
extern DAC_HandleTypeDef    DAC_1_Handler;

/**********************复位变量**************************************/
 void Reset_VAR(void)
{
  HRTIM1->sCommonRegs.ODISR	|= HRTIM_OUTPUT_TA1;	
  HRTIM1->sCommonRegs.ODISR	|= HRTIM_OUTPUT_TA2;
  HRTIM1->sCommonRegs.ODISR	|= HRTIM_OUTPUT_TC1;	
  HRTIM1->sCommonRegs.ODISR	|= HRTIM_OUTPUT_TC2;
	flag_Soft_start =Run;  //初始化FLAG
	flag_start_cnt  =Run;  //初始化FLAG 
	ERROR_flag      =STOP; //初始化FLAG  
	Data_update_flag=STOP;  //初始化FLAG  
	
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].PERxR=Min_period;
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR=Min_period>>1;//更新占空比

  PFC_voltage     =0.0f;  //PFC升压390V
  PFC_current     =0.0f;  //PFC电感电流
  AC_peak_voltage =0.0f;  //AC交流输入电压瞬时值
  AC_RMS          =0.0f;  //AC交流输入有效值
  Vout            =0.0f;  //LLC输出电压
  Iout            =0.0f;  //LLC输出电流

  PFC_ovp         =440.0f; //输入过压保护点
  LLC_ocp         =2.2f;   //输出电流过流保护点	
  AC_OVP          =275.0f; //交流电压过压保护点
  AC_UVP          =75.0f;  //交流电压过压保护点
	AC_OCP          =1.8f;   //交流电压过流保护点
	LLC_ovp         =27.0f;   //输出过压保护点
	
  OCP_CNT1        =0;     //OCP过流次数计数器
	OVP_CNT         =0;     //OVP过压次数计数器
	ACRMS_CNT       =0;     //清零
	ACRMS_CNT1      =0;     //清零
	PC_command[0]=0;        //清零
	PC_command[1]=0;        //清零
	PC_command[2]=0;        //清零
	PC_command[3]=0;        //清零
}

/**********************系统任务**************************************/
typedef enum
{
 Task_0_Initial_state, //初始状态
 Task_1_Vac_detc,      //检测输入电压
 Task_2_Vbus_detc,     //检测输入电压
 Task_3_Iac_detc,      //检测输出电压
 Task_4_Vout_detc,     //检测输出电压
 Task_5_Iout_detc,     //检测输出电压
 Task_6_PC_command,    //检测输出电压
 Task_7_Soft_start     //开始缓启动
}System_Task;

System_Task Current_State,Next_State; 

void state_machine(void)//状态机
{
	Next_State=Current_State=Task_0_Initial_state;
	while(1)
	{	 
		 Current_State=Next_State;
		 switch(Current_State)
			{
				case Task_0_Initial_state://初始状态
													{	 
                             Reset_VAR();										 //复位变量
														 Red_ON();                       //亮红灯
														 PID_INT();                      //PID补偿器初始化
                             type_3_int();                   //初始化LLC补偿器
                             Delay_ms(1000);                 //延时		 
									 					 Next_State=Task_1_Vac_detc;		 //下一状态：读取ADC值										
													}  
													break;
				case Task_1_Vac_detc://检测交流输入电压
													{	
                            if(Data_update_flag==STOP){Next_State = Task_1_Vac_detc;break;}
                            Data_update_flag=STOP;														
														if (AC_RMS > AC_OVP) //过压
														{
															flag_Vin_ovp = Error;			   
															Next_State = Task_0_Initial_state; 
															break;
														}
														if (flag_Vin_ovp == Error)//过压中
														{
															if (AC_RMS <AC_OVP-20.0f)flag_Vin_ovp = Normal;
															else{Next_State = Task_0_Initial_state;break;} 
														}
														if (AC_RMS < AC_UVP)//欠压
														{
															flag_Vin_uvp = Error; 
															Next_State = Task_0_Initial_state;
															break;
														}
														if (flag_Vin_uvp == Error)//欠压中
														{
															if (AC_RMS > AC_UVP+20.0f)flag_Vin_uvp = Normal;
															else{Next_State = Task_0_Initial_state;break;}
														}
														Next_State = Task_2_Vbus_detc;
													}
													break;	
				case Task_2_Vbus_detc : //检测PFC输出电压 
													{																	  
													  if(PFC_voltage>PFC_ovp){Next_State=Task_0_Initial_state;break;}//过压了
														Next_State=Task_3_Iac_detc;																																																		 												
													}					
													break;			
				case Task_3_Iac_detc : //检测PFC电流
													{																	  
													  if(PFC_current>AC_OCP){Next_State=Task_0_Initial_state;break;}//过流了
														Next_State=Task_4_Vout_detc;																																																		 												
													}					
													break;		
				case Task_4_Vout_detc : //检测LLC输出电压
													{
                            if(Vout>LLC_ovp)OVP_CNT++;
														else OVP_CNT=0;
														if(OVP_CNT>500){Next_State=Task_0_Initial_state;break;} 
														if(Vref_LLC>5.6f)
														{
															//次级60us内没有发送数据，异常了，需要关机
														  if(TIM2->CNT>50||Vout<3.6){Next_State=Task_0_Initial_state;break;} 
														}
                           	Next_State=Task_5_Iout_detc;																																																														 												
													}					
													break;	
				case Task_5_Iout_detc : //检测LLC输出电流
													{
                            if(Iout>LLC_ocp)OCP_CNT1++;
														else OCP_CNT1=0;
                            if(OCP_CNT1>2){Next_State=Task_0_Initial_state;break;} 
                            else 		Next_State=Task_6_PC_command;																																																															 												
													}					
													break;			
				case Task_6_PC_command : //检测上位机指令
													{
                            if(flag_start_cnt!=STOP)	Next_State=Task_7_Soft_start ; 
                            else 		Next_State=Task_1_Vac_detc;																																																														 												
													}					
													break;														
				case Task_7_Soft_start ://缓启动
													{
														Green_ON();//系统指示	
                            flag_start_cnt=STOP;
														HRTIM1->sCommonRegs.OENR |= HRTIM_OUTPUT_TC1;//使能输出
                            Vref_PFC=AC_RMS*1.414f;	//PFC输出，从当前电解电容电压开始											
													  Next_State=Task_1_Vac_detc;													
													}														
													break;													
				default:          break;
			}
	}
}
  
void HRTIM1_TIMC_IRQHandler(void)
{
if((HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].TIMxDIER & HRTIM_TIM_IT_REP) == HRTIM_TIM_IT_REP)
{
 HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].TIMxICR = HRTIM_TIM_IT_REP;
 __HAL_DMA_CLEAR_FLAG(&ADC1_DMA_Handler, DMA_FLAG_TC1);//清除标志位
 if((COMP2->CSR&0x40000000)!=0x40000000&&flag_start_cnt==STOP&&Vref_PFC>=390)//比较器输出低电平，未发生过流保护
 {
	__HAL_COMP_COMP2_EXTI_ENABLE_IT(); //使能比较器过流中断
	HRTIM1->sCommonRegs.OENR |= HRTIM_OUTPUT_TA1;//使能输出
	HRTIM1->sCommonRegs.OENR |= HRTIM_OUTPUT_TA2;//使能输出
 }
 Data_update_flag=Run;
// PA0   ADC1_IN1   PFC_CURRENT
// PA1   ADC1_IN2   PFC_VOLTAGE
// PA2   ADC1_IN3   交流电压输入
 PFC_current       =(float)((ADC1_RESULT[0]*3300)>>12)*0.0017f;//化简：R22/R23=100KΩ/5.1KΩ，采样电阻30毫欧，注意单位A
 PFC_voltage       =(float)((ADC1_RESULT[1]*3300)>>12)*0.137f;//化简：1+680KΩ*2/R37=1+1360/10=137,注意单位V
 AC_peak_voltage=((ADC1_RESULT[2]*3300)>>12)*0.001f;
//测量有效值，1000个点的平方和开根号
 ACRMS_CNT++;
 if(ACRMS_CNT>=3){ACRMS_CNT=0;ACRMS_CNT1+=1;AC_RMS2+=AC_peak_voltage*AC_peak_voltage;}//mV转V
 if(ACRMS_CNT1>=999){AC_RMS=137.0f*sqrtf(AC_RMS2*0.001f);ACRMS_CNT1=0;AC_RMS2=0.0f;USART3->TDR=AC_RMS*0.98f;}
//处理次级发过来的电压电流数据，见485.c文件
 Vout=(((((uint16_t)PC_command[1]<<6)|PC_command[0])*3300)>>12)*0.011f;//R93/R94+1=10+1;    //mV转V
 Iout=(((((uint16_t)PC_command[3]<<6)|PC_command[2])*3300)>>12)*0.0102f;  //R86/R87=19.6078431，采样电阻5m欧
//处理PFC+LLC环路
 if(flag_start_cnt==STOP)
	 {
		PID_loop(PFC_voltage,PFC_current);//计算PFC电压外环+电流内环
    if(Vref_PFC>=390)type_3_cal(Vout);//PFC输出电压到达390V，可以进行LLC计算了
		if(flag_Soft_start!=STOP)//缓启动，PFC输出电压爬坡到390V，防止大电流过冲等
		 {		
			 	 if(Vref_PFC<390)
				 {
					 Vref_PFC+=0.001f;//PFC输出电压每次累[加1mV
					 PFC_duty_max_start+=0.0001f;
			     if(PFC_duty_max_start>0.8f)PFC_duty_max_start=0.8f;//启动过程限制PFC冲击电流
				 }
         else if(Vref_LLC<21.0)//LLC输出电压爬坡到21V，改变该值可以改变LLC输出电压19~23V
				 {
					 TIM2->CNT=0;
					 Vref_LLC+=0.0001f;	//LLC输出电压每次累[加1mV
					 //限制LLC谐振回路电流峰值，减小冲击电流
					 //电流传感器0.132V代表1A电流，中心值1.65V，参考器件手册
					 HAL_DAC_SetValue(&DAC_1_Handler,DAC_CHANNEL_1,DAC_ALIGN_12B_R,2448);//阈值可以适当取大一些
				 }					 
         else flag_Soft_start=STOP;//PFC+LLC启动完毕					 			 
		 }
	 }
	}
}

void COMP2_IRQHandler(void)
{
  uint32_t extiline = COMP_GET_EXTI_LINE(COMP2);
  if (COMP_EXTI_GET_FLAG(extiline) != RESET)
  {
    COMP_EXTI_CLEAR_FLAG(extiline);
		__HAL_COMP_COMP2_EXTI_DISABLE_IT();//关闭比较器中断
    HRTIM1->sCommonRegs.ODISR	|= HRTIM_OUTPUT_TA1;	//关闭PWM输出
		HRTIM1->sCommonRegs.ODISR	|= HRTIM_OUTPUT_TA2;  //关闭PWM输出
  }
}



