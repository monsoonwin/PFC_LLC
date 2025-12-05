#ifndef __RS485_H
#define __RS485_H
#include "common.h"

/****************************************************************************************
  * @file    rs485.h
  * @author  尖叫的变压器
  * @version V1.0.0
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#define Power_board_address         0xAA       //电源板地址

void USART3_INT(void);
void Send_data_to_PC(UART_HandleTypeDef *huart,uint32_t *pData,uint32_t count_data);

#endif
