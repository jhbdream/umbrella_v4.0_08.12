#ifndef __USART_H
#define __USART_H
#include "sys.h"
#include "stdio.h"	 
 
#define USART_REC_LEN  			100  	//�����������ֽ��� 200
#define USART_LEN                   20
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1�
#define STATUS_OK			0x00
#define STATUS_ERR    0x01
extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u8  USART2_RX_BUF[USART_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u8  USART3_RX_BUF[USART_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u8  USART4_RX_BUF[USART_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u8  USART5_RX_BUF[USART_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u8 Rx2Flag;
extern u8 Rx3Flag;
extern u8 Rx4Flag;
extern u8 Rx5Flag;
extern u8 Uart2RxDataConut;
extern u8 Uart3RxDataConut;
extern u8 Uart4RxDataConut;
extern u8 Uart5RxDataConut;
extern u16 USART_RX_STA;         		//����״̬���	
//����봮���жϽ��գ��벻Ҫע�����º궨��


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
















