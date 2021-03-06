#ifndef __USART_H
#define __USART_H
#include "sys.h"
#include "stdio.h"	 
 
#define USART_REC_LEN  			100  	//定义最大接收字节数 200
#define USART_LEN                   20
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1�
#define STATUS_OK			0x00
#define STATUS_ERR    0x01
extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u8  USART2_RX_BUF[USART_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u8  USART3_RX_BUF[USART_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u8  USART4_RX_BUF[USART_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u8  USART5_RX_BUF[USART_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u8 Rx2Flag;
extern u8 Rx3Flag;
extern u8 Rx4Flag;
extern u8 Rx5Flag;
extern u8 Uart2RxDataConut;
extern u8 Uart3RxDataConut;
extern u8 Uart4RxDataConut;
extern u8 Uart5RxDataConut;
extern u16 USART_RX_STA;         		//接收状态标记	
//如果想串口中断接收，请不要注释以下宏定义


void uart_init(u32 pclk2,u32 bound);
void CLR_Buf(void);
void USART_SendString(char* s);
void USART_send(u8 ch);
void uart2_init(u32 pclk2,u32 bound);
void uart3_init(u32 pclk2,u32 bound);
void uart4_init(u32 pclk2,u32 bound);
void uart5_init(u32 pclk2,u32 bound);
void uart_init(u32 pclk2,u32 bound);


#endif	   
















