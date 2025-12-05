/****************************************************************************************
  * @file    HRTIM.c
  * @author  尖叫的变压器
  * @version V1.0.0
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#include "HRTIM.h"

HRTIM_HandleTypeDef  HRTIM1_structure;
__IO uint16_t PWM_TEST=20340;
//关于HRTIM参数讲解，可以参考视哔哩哔哩通用频教程，F3,G4系列通用
void HRTIM_INT(void)
{
	GPIO_InitTypeDef           GPIO_InitStruct;                           //GPIO参数描述结构体
	HRTIM_TimeBaseCfgTypeDef   pTimeBaseCfg  = {0};                       //HRTIM定时器基本参数描述结构体
  HRTIM_TimerCfgTypeDef      pTimerCfg     = {0};                       //HRTIM波形参数配置结构体
  HRTIM_OutputCfgTypeDef     pOutputCfg    = {0};                       //PWM输出有关参数描述结构体
	HRTIM_CompareCfgTypeDef    pCompareCfg   = {0};                       //PWM脉宽参数描述结构体  
	HRTIM_DeadTimeCfgTypeDef   pDeadTimeCfg  = {0};                       //死区时间描述结构体
	HRTIM_ADCTriggerCfgTypeDef pADCTriggerCfg= {0};                       //ADC触发选项配置
	__HAL_RCC_HRTIM1_CLK_ENABLE();                                        //开启HRTIM1时钟
	__HAL_RCC_GPIOA_CLK_ENABLE();                                         //开启GPIOA时钟
	
	
  HRTIM1_structure.Instance = HRTIM1;                                   //调用HRTIM1
  HRTIM1_structure.Init.HRTIMInterruptResquests =HRTIM_IT_NONE;// HRTIM_IT_FLT3;        //中断需求描述
  HRTIM1_structure.Init.SyncOptions = HRTIM_SYNCOPTION_NONE;            //HRTIM是否和外部定时器同步
  HAL_HRTIM_Init(&HRTIM1_structure);                                    //初始化HRTIM1
	
  HAL_HRTIM_DLLCalibrationStart(&HRTIM1_structure, HRTIM_CALIBRATIONRATE_7300);//矫正
  HAL_HRTIM_PollForDLLCalibration(&HRTIM1_structure, 10);                      //矫正
	
 /*ADC触发信号********************************************************************************************************************************/	
	pADCTriggerCfg.Trigger=HRTIM_ADCTRIGGEREVENT13_TIMERC_CMP2;             //HRTIM中的定时器TIMA的Compare2作为触发源
	pADCTriggerCfg.UpdateSource=HRTIM_ADCTRIGGERUPDATE_TIMER_C;             //TIMA复位时可以更新触发源
	HAL_HRTIM_ADCTriggerConfig(&HRTIM1_structure,HRTIM_ADCTRIGGER_1,&pADCTriggerCfg);//ADC触发初始化
  //TIMEA定时器基本参数
  pTimeBaseCfg.Period = Min_period;                                     //PWM周期, 4.608G/200000=23040
  pTimeBaseCfg.RepetitionCounter = 0x03;                                //重复多少个周期，发生一次中断
  pTimeBaseCfg.PrescalerRatio = HRTIM_PRESCALERRATIO_MUL16;             //32倍频   ，144MHz*32=4.608Ghz
  pTimeBaseCfg.Mode = HRTIM_MODE_CONTINUOUS;                            //连续计数，周而复始
	HAL_HRTIM_TimeBaseConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_A, &pTimeBaseCfg);//初始化
	
  pTimeBaseCfg.Period = PWM_PERIOD;                                     //PWM周期, 4.608G/200000=23040
  pTimeBaseCfg.RepetitionCounter = 0x03;                                //重复多少个周期，发生一次中断
  pTimeBaseCfg.PrescalerRatio = HRTIM_PRESCALERRATIO_MUL16;             //32倍频   ，144MHz*32=4.608Ghz
  pTimeBaseCfg.Mode = HRTIM_MODE_CONTINUOUS;                            //连续计数，周而复始
	HAL_HRTIM_TimeBaseConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_C, &pTimeBaseCfg);//初始化
	
  pTimerCfg.InterruptRequests = HRTIM_TIM_IT_NONE;                       //中断需求描述
  pTimerCfg.DMARequests = HRTIM_MASTER_DMA_NONE;                        //DMA需求 
  pTimerCfg.DMASrcAddress = 0x0000;                                     //数据来源
  pTimerCfg.DMADstAddress = 0x0000;                                     //数据目的地
  pTimerCfg.DMASize = 0x1;                                              //搬运多少个数
  pTimerCfg.HalfModeEnable = HRTIM_HALFMODE_DISABLED;                   //HALF模式关闭
  pTimerCfg.StartOnSync = HRTIM_SYNCSTART_ENABLED;                      //同步开始
  pTimerCfg.ResetOnSync = HRTIM_SYNCRESET_ENABLED;                      //同步复位
  pTimerCfg.DACSynchro = HRTIM_DACSYNC_NONE;                            //同步DAC
  pTimerCfg.PreloadEnable = HRTIM_PRELOAD_ENABLED;                      //预加载功能关闭
  pTimerCfg.UpdateGating = HRTIM_UPDATEGATING_INDEPENDENT;              //更新事件，独立发生
  pTimerCfg.BurstMode = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;             //促发模式关闭
  pTimerCfg.RepetitionUpdate = HRTIM_UPDATEONREPETITION_ENABLED;        //重复计数更新事件功能关闭
	pTimerCfg.PushPull = HRTIM_TIMPUSHPULLMODE_DISABLED;                  //关闭推挽模式
	pTimerCfg.FaultEnable = HRTIM_TIMFAULTENABLE_NONE;                    //使能故障1通道
	pTimerCfg.FaultLock = HRTIM_TIMFAULTLOCK_READWRITE;                   //故障状态可读写
	pTimerCfg.DeadTimeInsertion = HRTIM_TIMDEADTIMEINSERTION_ENABLED;     //使能死区
	pTimerCfg.DelayedProtectionMode = HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED;//延时保护关闭
	pTimerCfg.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_TIMER_A;             //选择事件用于定时器寄存器更新
	pTimerCfg.ResetTrigger = HRTIM_TIMRESETTRIGGER_UPDATE;                //复位更新事件
  pTimerCfg.ResetUpdate = HRTIM_TIMUPDATEONRESET_ENABLED;               //定时器复位时，更新事件
  HAL_HRTIM_WaveformTimerConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_A, &pTimerCfg);//初始化
	
	pTimerCfg.InterruptRequests = HRTIM_TIM_IT_REP;                       //中断需求描述
	pTimerCfg.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_TIMER_C;             //选择事件用于定时器寄存器更新
	HAL_HRTIM_WaveformTimerConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_C, &pTimerCfg);//初始化
	
  pCompareCfg.CompareValue = Min_period>>1;
	pCompareCfg.AutoDelayedMode = HRTIM_AUTODELAYEDMODE_REGULAR;
  HAL_HRTIM_WaveformCompareConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, &pCompareCfg);
 
  pCompareCfg.CompareValue = 1000;
	pCompareCfg.AutoDelayedMode = HRTIM_AUTODELAYEDMODE_REGULAR;
  HAL_HRTIM_WaveformCompareConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_1, &pCompareCfg);	
	
  pCompareCfg.AutoDelayedTimeout = 500;
  HAL_HRTIM_WaveformCompareConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_2, &pCompareCfg);
  HAL_HRTIM_WaveformCompareConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_3, &pCompareCfg);
  HAL_HRTIM_WaveformCompareConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_4, &pCompareCfg);
	//死区时间配置
	pDeadTimeCfg.Prescaler=HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV1;           //fHRTIM=144MHz
	pDeadTimeCfg.RisingValue=300;                                            //时间
	pDeadTimeCfg.RisingSign=HRTIM_TIMDEADTIME_FALLINGSIGN_POSITIVE;         //下降沿为正值
	pDeadTimeCfg.RisingLock=HRTIM_TIMDEADTIME_RISINGLOCK_READONLY;          //死区时间上升沿只读模式
  pDeadTimeCfg.RisingSignLock = HRTIM_TIMDEADTIME_RISINGSIGNLOCK_WRITE;   //可写
	pDeadTimeCfg.FallingValue=300;                                           //下降沿死区时间
  pDeadTimeCfg.FallingSign = HRTIM_TIMDEADTIME_FALLINGSIGN_POSITIVE;      //死区时间为正值
	pDeadTimeCfg.FallingLock=HRTIM_TIMDEADTIME_FALLINGLOCK_READONLY;        //死区时间下降沿只读模式
	pDeadTimeCfg.FallingSignLock=HRTIM_TIMDEADTIME_FALLINGSIGNLOCK_READONLY;//死区下降沿锁定
	HAL_HRTIM_DeadTimeConfig(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_A,&pDeadTimeCfg);//初始化
	
  pOutputCfg.Polarity = HRTIM_OUTPUTPOLARITY_HIGH;                      //定时器输出极行高
  pOutputCfg.SetSource = HRTIM_OUTPUTSET_TIMPER;                        //定时器计数回到零时，PWM输出拉高
  pOutputCfg.ResetSource = HRTIM_OUTPUTRESET_TIMCMP1;                   //定时器计数到Compare1时，PWM输出低
  pOutputCfg.IdleMode = HRTIM_OUTPUTIDLEMODE_NONE;                      //空闲模式不动作
  pOutputCfg.IdleLevel = HRTIM_OUTPUTIDLELEVEL_INACTIVE;                //空闲模式输出无效
  pOutputCfg.FaultLevel = HRTIM_OUTPUTFAULTLEVEL_NONE;                 //错误模式，输出高阻状态
  pOutputCfg.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED;      //斩波模式关闭
  pOutputCfg.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;//初始化
  HAL_HRTIM_WaveformOutputConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1, &pOutputCfg);//初始化
  HAL_HRTIM_WaveformOutputConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA2, &pOutputCfg);//初始化
  HAL_HRTIM_WaveformOutputConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_C, HRTIM_OUTPUT_TC1, &pOutputCfg);//初始化
  HAL_HRTIM_WaveformOutputConfig(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_C, HRTIM_OUTPUT_TC2, &pOutputCfg);//初始化

	HAL_HRTIM_SimpleBaseStart(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_A);  //初始化
	HAL_HRTIM_SimpleBaseStart(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_C);  //初始化
	
	Delay_us(300);
	HAL_HRTIM_SimpleOCStart(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_A,HRTIM_OUTPUT_TA1);//初始化
	HAL_HRTIM_SimpleOCStart(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_A,HRTIM_OUTPUT_TA2);//初始化
	
	HAL_HRTIM_SimplePWMStart(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_A,HRTIM_OUTPUT_TA1);//初始化
	HAL_HRTIM_SimplePWMStart(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_A,HRTIM_OUTPUT_TA2);//初始化
	
	HAL_HRTIM_SimpleOCStart(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_C,HRTIM_OUTPUT_TC1);//初始化
	HAL_HRTIM_SimplePWMStart(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_C,HRTIM_OUTPUT_TC1);//初始化
	
	__HAL_HRTIM_TIMER_ENABLE_IT(&HRTIM1_structure,HRTIM_TIMERINDEX_TIMER_C,HRTIM_TIM_IT_REP);//使能重复计数中断
	HAL_NVIC_SetPriority(HRTIM1_TIMC_IRQn,1,1); //抢占优先级为0，子优先级为0
  HAL_NVIC_EnableIRQ(HRTIM1_TIMC_IRQn);       //使能中断线
	
	__HAL_HRTIM_ENABLE(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_A);       //使能
	__HAL_HRTIM_ENABLE(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_C);       //使能
	
	HAL_HRTIM_WaveformSetOutputLevel(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1, HRTIM_OUTPUTLEVEL_ACTIVE);//输出有效电平状态
	HAL_HRTIM_WaveformSetOutputLevel(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA2, HRTIM_OUTPUTLEVEL_ACTIVE);//输出有效电平状态
	HAL_HRTIM_WaveformSetOutputLevel(&HRTIM1_structure, HRTIM_TIMERINDEX_TIMER_C, HRTIM_OUTPUT_TC1, HRTIM_OUTPUTLEVEL_ACTIVE);//输出有效电平状态

	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = Min_period>>1;//输出PWM50%占空比
	
	HRTIM1->sCommonRegs.ODISR	|= HRTIM_OUTPUT_TC1;	//关闭PWM输出
	HRTIM1->sCommonRegs.ODISR	|= HRTIM_OUTPUT_TA1;	//关闭PWM输出
	HRTIM1->sCommonRegs.ODISR	|= HRTIM_OUTPUT_TA2;	//关闭PWM输出
	
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;                          //管脚描述
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;                               //复用推挽模式
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;                                 //下拉
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;                         //IO口速度设置为最高
	GPIO_InitStruct.Alternate = GPIO_AF13_HRTIM1;                         //复用为HRTIM1，重映射，芯片手册有描述
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                               //IO初始化
	
	GPIO_InitStruct.Pin = GPIO_PIN_12;                                    //管脚描述
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);                               //IO初始化
	
}


