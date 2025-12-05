#include "delay.h"
/****************************************************************************************
  * @file    delay.c
  * @author  尖叫的变压器
  * @version V1.0.0
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/
void Delay_ms(__IO uint32_t nCount)
{
  while (nCount != 0)
  {
		nCount--;
	  Delay_us(1000);
  }
}
void Delay_us(__IO uint16_t time)
{
		 TIM16->CNT=0;			
	   while((TIM16->CNT)<time);   //TIME6,时钟频率为1MHz，计一个数为1us  
}
/*********************************************************************************************************************/
//关于定时器参数讲解，可以参考视哔哩哔哩通用频教程，F0,F1,F3,F4,F7,G0,G4系列通用
void Init_TIM_Basic(TIM_TypeDef *TIM_X)                                                         //初始化定时器16
{
	TIM_HandleTypeDef TIM_HandleTypeDef_Structure;
	__HAL_RCC_TIM16_CLK_ENABLE();

  TIM_HandleTypeDef_Structure.Instance = TIM_X;
  TIM_HandleTypeDef_Structure.Init.Prescaler = 72000000/1000000-1;
  TIM_HandleTypeDef_Structure.Init.CounterMode = TIM_COUNTERMODE_UP;
  TIM_HandleTypeDef_Structure.Init.Period = 0xFFFF; 
  TIM_HandleTypeDef_Structure.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  TIM_HandleTypeDef_Structure.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&TIM_HandleTypeDef_Structure);
	__HAL_TIM_ENABLE(&TIM_HandleTypeDef_Structure);
}
//关于定时器参数讲解，可以参考视哔哩哔哩通用频教程，F0,F1,F3,F7,G0,G4系列通用
void TIM2_INT(void)
{
	TIM_HandleTypeDef TIM_HandleTypeDef_Structure;
	__HAL_RCC_TIM2_CLK_ENABLE();

  TIM_HandleTypeDef_Structure.Instance = TIM2;
  TIM_HandleTypeDef_Structure.Init.Prescaler = 72000000/1000000-1;
  TIM_HandleTypeDef_Structure.Init.CounterMode = TIM_COUNTERMODE_UP;
  TIM_HandleTypeDef_Structure.Init.Period = 0xFFFFFFFF; 
  TIM_HandleTypeDef_Structure.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  TIM_HandleTypeDef_Structure.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&TIM_HandleTypeDef_Structure);
	__HAL_TIM_ENABLE(&TIM_HandleTypeDef_Structure);
}
