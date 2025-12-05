/****************************************************************************************
  * @file    ADC.c
  * @author  尖叫的变压器
  * @version V1.0.0
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#include "ADC.h"

/*
ADC检测IO配置
              
 PA0   ADC1_IN1      模拟    PFC_CURRENT
 PA1   ADC1_IN2      模拟    PFC_VOLTAGE
 PA2   ADC1_IN3      模拟    交流电压输入
 
*/

ADC_HandleTypeDef  ADC1_Handler;      //ADC1句柄
DMA_HandleTypeDef  ADC1_DMA_Handler;  //ADC1的DMA句柄

__IO uint16_t ADC1_RESULT[3]={0};
//关于ADC参数讲解，可以参考视哔哩哔哩通用频教程，F0,F1,F3,F4,F7,G0,G4系列通用
void ADC1_Init(void)
{
	GPIO_InitTypeDef       GPIO_Initure;                                       //定义IO口相关结构体
	ADC_ChannelConfTypeDef ADC1_ChanConf;                                      //定义ADC1相关结构体
	ADC_MultiModeTypeDef   multimode ;                                         //定义ADC模式相关结构体
	
  __HAL_RCC_ADC12_CLK_ENABLE();                                              //使能ADC12时钟
  __HAL_RCC_GPIOA_CLK_ENABLE(); 																						 //开启GPIOA时钟
	__HAL_RCC_DMA1_CLK_ENABLE();                                               //DMA1时钟使能
	
  GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2; 												 //PA0/PA1/PA2
  GPIO_Initure.Mode=GPIO_MODE_ANALOG; 																		   //模拟
  GPIO_Initure.Pull=GPIO_NOPULL; 																						 //不带上下拉
  HAL_GPIO_Init(GPIOA,&GPIO_Initure);                                        //GPIOA初始化

	ADC1_Handler.Instance=ADC1;                                                //选中ADC1
	ADC1_Handler.Init.ClockPrescaler=ADC_CLOCK_SYNC_PCLK_DIV1;                 //4分频，ADCCLK=PCLK2=72MHZ
	ADC1_Handler.Init.Resolution=ADC_RESOLUTION_12B; 													 //12位模式
	ADC1_Handler.Init.DataAlign=ADC_DATAALIGN_RIGHT;													 //右对齐
	ADC1_Handler.Init.ScanConvMode=ENABLE; 															       //非扫描模式
	ADC1_Handler.Init.EOCSelection=DISABLE; 																	 //关闭EOC中断
	ADC1_Handler.Init.ContinuousConvMode=ENABLE; 														   //关闭连续转换
	ADC1_Handler.Init.NbrOfConversion=3; 																       //1个转换在规则序列中
	ADC1_Handler.Init.DiscontinuousConvMode=DISABLE; 													 //禁止不连续采样模式
	ADC1_Handler.Init.NbrOfDiscConversion=0; 																	 //不连续采样通道数为0
	ADC1_Handler.Init.ExternalTrigConv=ADC_EXTERNALTRIGCONVHRTIM_TRG1;         //ADC触发源选择
	ADC1_Handler.Init.ExternalTrigConvEdge=ADC_EXTERNALTRIGCONVEDGE_RISING;		 //上升沿触发
	ADC1_Handler.Init.DMAContinuousRequests=ENABLE; 													 //使能DMA请求
	ADC1_Handler.Init.LowPowerAutoWait = DISABLE;                              //关闭低功耗模式
  ADC1_Handler.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;                      //数据溢出覆盖
	HAL_ADC_Init(&ADC1_Handler);																							 //初始化
	
	multimode.Mode = ADC_MODE_INDEPENDENT;                                     //ADC相互独立运行
  HAL_ADCEx_MultiModeConfigChannel(&ADC1_Handler, &multimode);               //多个ADC运行配置
	
	ADC1_ChanConf.Channel=ADC_CHANNEL_1;                                       //通道
	ADC1_ChanConf.Rank=ADC_REGULAR_RANK_1;                                     //第1个序列
	ADC1_ChanConf.SamplingTime=ADC_SAMPLETIME_181CYCLES_5;                       //采样时间
	ADC1_ChanConf.OffsetNumber=ADC_OFFSET_NONE;                                //无偏移数量
	ADC1_ChanConf.Offset = 0;                                                  //偏移量=0
	ADC1_ChanConf.SingleDiff=ADC_SINGLE_ENDED;                                 //单端模式
	HAL_ADC_ConfigChannel(&ADC1_Handler,&ADC1_ChanConf);                       //通道配置
	
	ADC1_ChanConf.Channel=ADC_CHANNEL_2;                                       //通道
	ADC1_ChanConf.Rank=ADC_REGULAR_RANK_2;                                     //第2个序列
	ADC1_ChanConf.SamplingTime=ADC_SAMPLETIME_181CYCLES_5;                      //采样时间
	ADC1_ChanConf.OffsetNumber=ADC_OFFSET_NONE;                                //无偏移数量
	ADC1_ChanConf.Offset = 0;                                                  //偏移量=0
	ADC1_ChanConf.SingleDiff=ADC_SINGLE_ENDED;                                 //单端模式
	HAL_ADC_ConfigChannel(&ADC1_Handler,&ADC1_ChanConf);                       //通道配置
	
	ADC1_ChanConf.Channel=ADC_CHANNEL_3;                                       //通道
	ADC1_ChanConf.Rank=ADC_REGULAR_RANK_3;                                     //第3个序列
	ADC1_ChanConf.SamplingTime=ADC_SAMPLETIME_181CYCLES_5;                      //采样时间
	ADC1_ChanConf.OffsetNumber=ADC_OFFSET_NONE;                                //无偏移数量
	ADC1_ChanConf.Offset = 0;                                                  //偏移量=0
	ADC1_ChanConf.SingleDiff=ADC_SINGLE_ENDED;                                 //单端模式
	HAL_ADC_ConfigChannel(&ADC1_Handler,&ADC1_ChanConf);                       //通道配置
	
	HAL_ADCEx_Calibration_Start(&ADC1_Handler, ADC_SINGLE_ENDED);              //ADC矫正

	ADC1_DMA_Handler.Instance = DMA1_Channel1;                                 //使用DMA1的通道1
  ADC1_DMA_Handler.Init.Direction = DMA_PERIPH_TO_MEMORY;                    //数据传输方向：外设到内存
  ADC1_DMA_Handler.Init.PeriphInc = DMA_PINC_DISABLE;                        //外设地址不增加
  ADC1_DMA_Handler.Init.MemInc = DMA_MINC_ENABLE;                            //内存地址增加
  ADC1_DMA_Handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;       //外设数据宽度
  ADC1_DMA_Handler.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;          //外设数据对齐方式
  ADC1_DMA_Handler.Init.Mode = DMA_CIRCULAR;                                 //循环传输
  ADC1_DMA_Handler.Init.Priority = DMA_PRIORITY_VERY_HIGH ;									 //DMA抢占优先级最高
	HAL_DMA_Init(&ADC1_DMA_Handler);								                           //初始化
	__HAL_LINKDMA(&ADC1_Handler,DMA_Handle,ADC1_DMA_Handler);                  //把结构体的参数赋给DMA
	
	HAL_ADC_Start_DMA(&ADC1_Handler,(uint32_t*)ADC1_RESULT,3);                 //初始化ADC采样
	
	HAL_ADC_Start(&ADC1_Handler);                                              //开启AD
}
