#include "Int.h"
/****************************************************************************************
  * @file    Int.c
  * @author  尖叫的变压器
  * @version V1.0.0
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/
void Initial_prepheral_(void)
{
	HAL_Init();               //库函数初始化
	SystemClock_Config_HSE(); //系统时钟初始化
	Init_TIM_Basic(TIM16);    //初始化定时器6, 16bit
	TIM2_INT();               //32位定时器初始化
	LED_GPIO_CONFIG();        //初始化LED IO口   
  COMP2_Config();           //比较器2初始化  
	ADC1_Init();              //ADC1初始化
	HRTIM_INT();              //高精度定时器初始化
	USART3_INT();             //串口初始化用于接收次级电压电流数据
  type_3_tustin();	        //提取LLC环路系数，仿真+实际调试获取
}

































