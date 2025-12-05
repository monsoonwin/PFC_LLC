#include "COMP.h"
/****************************************************************************************
  * @file    COMP.c
  * @author  尖叫的变压器
  * @version V1.0.0
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/
//比较器接口
//        COMP_2_IN+  接 ILpeak ，   PA7
//	      COMP_2_IN-  接 DAC1_OUT1   PA4      
DAC_HandleTypeDef      DAC_1_Handler;
COMP_HandleTypeDef     hcomp2;
void COMP2_Config(void)
{
	
	DAC_ChannelConfTypeDef DAC1_CH1;        //DAC1的句柄
	GPIO_InitTypeDef       GPIO_InitStruct; //GPIO结构体

  __HAL_RCC_GPIOA_CLK_ENABLE();			      //开启GPIOA时钟
	__HAL_RCC_SYSCFG_CLK_ENABLE();          //系统时钟
  __HAL_RCC_PWR_CLK_ENABLE();             //开启PWR时钟
	__HAL_RCC_DAC1_CLK_ENABLE();            //开机DAC1时钟
	
	//IN_P
	GPIO_InitStruct.Mode  = GPIO_MODE_ANALOG ;  //设定为模拟功能
  GPIO_InitStruct.Pull  = GPIO_NOPULL;        //无上下拉
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_4;//IO口选择
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);     //GPIOA初始化
	
	DAC_1_Handler.Instance = DAC1;	            //选择DAC1
	HAL_DAC_Init(&DAC_1_Handler);							  //DAC1初始化
	
  DAC1_CH1.DAC_Trigger           = DAC_TRIGGER_NONE;                  //不需要外部触发
  DAC1_CH1.DAC_OutputBuffer      = DAC_OUTPUTBUFFER_ENABLE;           //DAC输出缓冲器打开
  HAL_DAC_ConfigChannel(&DAC_1_Handler, &DAC1_CH1, DAC_CHANNEL_1);    //初始化
	
	DAC1_CH1.DAC_OutputSwitch = DAC_OUTPUTSWITCH_DISABLE;               //关闭通道2
  HAL_DAC_ConfigChannel(&DAC_1_Handler, &DAC1_CH1, DAC_CHANNEL_2);    //初始化
	HAL_DAC_Start(&DAC_1_Handler,DAC_CHANNEL_1); 											  //开启DAC通道1  
	HAL_DAC_SetValue(&DAC_1_Handler,DAC_CHANNEL_1,DAC_ALIGN_12B_R,2096);//DAC输出电压3700/4096*3300=2.98V
	
  hcomp2.Instance = COMP2;  //选择比较器2
  hcomp2.Init.InvertingInput = COMP_INVERTINGINPUT_DAC1_CH1;  //反向端接DAC1_CH1
  hcomp2.Init.NonInvertingInput = COMP_NONINVERTINGINPUT_IO1; //同相端接GPIO
  hcomp2.Init.Output = COMP_OUTPUT_NONE;//比较器输出连接到内部
  hcomp2.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;//非反向
  hcomp2.Init.BlankingSrce = COMP_BLANKINGSRCE_NONE;//不滤波
  hcomp2.Init.TriggerMode = COMP_TRIGGERMODE_IT_RISING;	//上升沿触发中断
  HAL_COMP_Init(&hcomp2);//初始化
	HAL_COMP_Start(&hcomp2);//启动
	
	HAL_NVIC_SetPriority(COMP2_IRQn, 0, 0);//抢占优先级为0，子优先级为0
  HAL_NVIC_EnableIRQ(COMP2_IRQn);	       //使能中断线
	__HAL_COMP_COMP2_EXTI_ENABLE_RISING_EDGE() ;//开启上升沿中断
	__HAL_COMP_COMP2_EXTI_ENABLE_IT();//使能中断
	HAL_COMP_Start(&hcomp2);//启动比较器
}




