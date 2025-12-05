#include "rs485.h"	
#include "stm32F3xx_hal.h"

/****************************************************************************************
  * @file    rs485.c
  * @author  尖叫的变压器
  * @version V1.0.0
	* @数字电源交流群  599937745
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/
__IO  uint32_t         Baud_rate_set    =2000000;

DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart3_tx;
UART_HandleTypeDef USART3_Handler;  	//USART3句柄(用于RS485)

uint8_t    PC_command[10] 	 ={0};
uint8_t    Send_to_PC[10]   ={0};


//串口通信IO配置
//	PB10   USART3_TX  		串口接发送           设置AF_7模式，输出上拉
//	PB11   USART3_RX  		串口发接收 			 		 设置AF_7模式，输出上拉

void USART3_INT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__HAL_RCC_GPIOB_CLK_ENABLE();							//使能GPIOA时钟
	__HAL_RCC_USART3_CLK_ENABLE();						//使能USART3时钟
	__HAL_RCC_DMA1_CLK_ENABLE();              //DMA1时钟使能

	
	//TX RX 收发IO口
	GPIO_InitStruct.Pin=GPIO_PIN_10|GPIO_PIN_11; 	//PB10,11
	GPIO_InitStruct.Mode=GPIO_MODE_AF_PP;				//复用推挽输出
	GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate=GPIO_AF7_USART3;		//复用为USART3
	HAL_GPIO_Init(GPIOB,&GPIO_InitStruct);	   		//初始化PA2,3
	
	//USART 初始化设置
	USART3_Handler.Instance=USART3;			        												  //USART3
	USART3_Handler.Init.BaudRate=Baud_rate_set ;		        							//波特率
	USART3_Handler.Init.WordLength=UART_WORDLENGTH_8B;										//字长为8位数据格式
	USART3_Handler.Init.StopBits=UART_STOPBITS_1;												  //一个停止位
	USART3_Handler.Init.Parity=UART_PARITY_NONE;													//无奇偶校验位
	USART3_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;										//无硬件流控
	USART3_Handler.Init.Mode=UART_MODE_TX_RX;		    										  //收发模式
	USART3_Handler.Init.OverSampling = UART_OVERSAMPLING_16;							//过采样率
	USART3_Handler.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;     //NA
	USART3_Handler.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT; //不使用高级功能
	HAL_UART_Init(&USART3_Handler);			        												  //HAL_UART_Init()会使能USART3

  __HAL_UART_CLEAR_FLAG(&USART3_Handler, UART_FLAG_RXNE);
	__HAL_UART_CLEAR_FLAG(&USART3_Handler, UART_CLEAR_TCF);
	__HAL_UART_ENABLE_IT(&USART3_Handler,UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&USART3_Handler,UART_IT_TC);
	
	HAL_NVIC_EnableIRQ(USART3_IRQn);
	HAL_NVIC_SetPriority(USART3_IRQn,0,0);
	
	//TX
	hdma_usart3_tx.Instance = DMA1_Channel2;                                    //USART3的发送通道DMA1_CH5
  hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;                       //数据传输方向：内存到外设
  hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;                           //外设地址不增加
  hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;                               //内存地址增加
  hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;              //外设数据宽度
  hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;                 //外设数据对齐方式
  hdma_usart3_tx.Init.Mode = DMA_NORMAL;                                      //单次发送完毕，结束
  hdma_usart3_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH ;											//DMA抢占优先级最高
	HAL_DMA_Init(&hdma_usart3_tx);								                              //初始化

	__HAL_LINKDMA(&USART3_Handler,hdmatx,hdma_usart3_tx);                       //把hdma_usart3_tx结构体的参数赋给DMA
}

//次级12位ADC数据发送到初级
//次级ADC采集的数据摆放格式：00+低6位
//次级ADC采集的数据摆放格式：01+高6位
//次级ADC采集的数据摆放格式：02+低6位
//次级ADC采集的数据摆放格式：03+高6位
//高六位+低六位=12位ADC数据
void USART3_IRQHandler(void)
{
	 uint8_t P_data=0;
   if((__HAL_UART_GET_IT(&USART3_Handler, UART_IT_RXNE) != RESET))
		 {
			  __HAL_UART_CLEAR_FLAG(&USART3_Handler, UART_FLAG_RXNE);
				P_data=USART3->RDR & 0x00FF;
			  PC_command[P_data>>6]=P_data&0x3F;
			 TIM2->CNT=0;
		 }
	 if((__HAL_UART_GET_IT(&USART3_Handler, UART_IT_TC) != RESET))
		 {
			 __HAL_UART_DISABLE_IT(&USART3_Handler,UART_IT_TC);
			 __HAL_UART_CLEAR_FLAG(&USART3_Handler, UART_CLEAR_TCF);
		 }
	 __HAL_UART_SEND_REQ(&USART3_Handler, UART_RXDATA_FLUSH_REQUEST); //清除接收数据非空中断标志
	 __HAL_UART_CLEAR_OREFLAG(&USART3_Handler); 
}

void Send_data_to_PC(UART_HandleTypeDef *huart,uint32_t *pData,uint32_t count_data)
{
	
	  CLEAR_BIT(USART3->CR3, USART_CR3_DMAT);//失能USART DMA发送
    __HAL_DMA_DISABLE(huart->hdmatx);      // Disable the peripheral
    DMA_SetConfig(huart->hdmatx, (uint32_t)pData, (uint32_t)&huart->Instance->TDR, count_data); /* Configure the source, destination address and the data length & clear flags*/
	 __HAL_UART_CLEAR_FLAG(&USART3_Handler, UART_CLEAR_TCF);
	 __HAL_UART_ENABLE_IT(&USART3_Handler,UART_IT_TC);
   __HAL_DMA_ENABLE(huart->hdmatx);        // Enable the Peripheral 
	 SET_BIT(USART3->CR3, USART_CR3_DMAT);   //使能USART DMA发送

}



