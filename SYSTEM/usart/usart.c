#include "usart.h"	  
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif
#define STATUS_OK			0x00
#define STATUS_ERR    0x01
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef�� d in stdio.h. */ 
FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc����
//printf�������ָ��fputc����fputc���������
//����ʹ�ô���1(USART1)���printf��Ϣ
int fputc(int ch, FILE *f)
{      
	while((USART2->SR&0X40)==0);//�ȴ���һ�δ������ݷ������  
	USART2->DR = (u8) ch;      	//дDR,����1����������
	return ch;
}
#endif 
//end
//////////////////////////////////////////////////////////////////
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
u8 USART2_RX_BUF[USART_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
u8 USART3_RX_BUF[USART_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
u8 USART4_RX_BUF[USART_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
u8 USART5_RX_BUF[USART_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.

u8 Uart2RxDataConut=0;
u8 Uart3RxDataConut=0;
u8 Uart4RxDataConut=0;
u8 Uart5RxDataConut=0;

u8 Rx2Flag = 0;
u8 Rx3Flag = 0;
u8 Rx4Flag = 0;
u8 Rx5Flag = 0;

unsigned char Cmd_Read_Id[8] = {0x01,0x08,0xa1,0x20,0x00,0x00,0x00,0x00};

//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	  
u8 First_Int =0; //��ǰ����������

//��ʼ��IO ����1
//pclk2:PCLK2ʱ��Ƶ��(Mhz)
//bound:������ 
void uart_init(u32 pclk2,u32 bound)
{  	 
	
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//�õ�USARTDIV
	mantissa=temp;				 //�õ���������
	fraction=(temp-mantissa)*16; //�õ�С������	 
    mantissa<<=4;
	mantissa+=fraction; 
	RCC->APB2ENR|=1<<2;   //ʹ��PORTA��ʱ��  
	RCC->APB2ENR|=1<<14;  //ʹ�ܴ���ʱ�� 
	GPIOA->CRH&=0XFFFFF00F;//IO״̬����
	GPIOA->CRH|=0X000008B0;//IO״̬���� 
	RCC->APB2RSTR|=1<<14;   //��λ����1
	RCC->APB2RSTR&=~(1<<14);//ֹͣ��λ	   	   
	//����������
 	USART1->BRR=mantissa; // ����������	 
	USART1->CR1|=0X200C;  //1λֹͣ,��У��λ.

	//ʹ�ܽ����ж� 
	USART1->CR1|=1<<5;    //���ջ������ǿ��ж�ʹ��	    	
	MY_NVIC_Init(0,0,USART1_IRQn,2);//��2��������ȼ� 
}


void uart2_init(u32 pclk2,u32 bound)
{
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//�õ�USARTDIV
	mantissa=temp;				 //�õ���������
	fraction=(temp-mantissa)*16; //�õ�С������	 
    mantissa<<=4;
	mantissa+=fraction; 
	RCC->APB2ENR|=1<<2;   //ʹ��PORTA��ʱ��  
	RCC->APB1ENR|=1<<17;  //ʹ�ܴ���ʱ�� 
	GPIOA->CRL&=0XFFFF00FF;//IO״̬����
	GPIOA->CRL|=0X00008B00;//IO״̬���� 
	RCC->APB1RSTR|=1<<17;   //��λ����1
	RCC->APB1RSTR&=~(1<<17);//ֹͣ��λ	   	   
	//����������
 	USART2->BRR=mantissa; // ����������	 
	USART2->CR1|=0X200C;  //1λֹͣ,��У��λ.

	//ʹ�ܽ����ж� 
	USART2->CR1|=1<<5;    //���ջ������ǿ��ж�ʹ��	    	
	MY_NVIC_Init(0,0,USART2_IRQn,2);//��2��������ȼ� 

}

void uart3_init(u32 pclk2,u32 bound)	//����3���� PD8 PD9
{
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//�õ�USARTDIV
	mantissa=temp;				 //�õ���������
	fraction=(temp-mantissa)*16; //�õ�С������	 
  mantissa<<=4;
	mantissa+=fraction; 
		
	RCC->APB2ENR|=1<<0;   //ʹ��AFIOʱ��
	RCC->APB2ENR|=1<<5;   //ʹ��PORTD��ʱ��  
	RCC->APB1ENR|=1<<18;  //ʹ�ܴ���ʱ�� 
	
	GPIOD->CRH&=0XFFFFFF00;//IO״̬����
	GPIOD->CRH|=0X0000008B;//IO״̬���� 
	AFIO->MAPR|=3<<4;		//����3��ȫ��ӳ��
	
	RCC->APB1RSTR|=1<<18;   //��λ����
	RCC->APB1RSTR&=~(1<<18);//ֹͣ��λ	   	   
	//����������
 	USART3->BRR=mantissa; // ����������	 
	USART3->CR1|=0X200C;  //1λֹͣ,��У��λ.

	//ʹ�ܽ����ж� 
	USART3->CR1|=1<<5;    //���ջ������ǿ��ж�ʹ��	    	
	MY_NVIC_Init(0,0,USART3_IRQn,2);//��2��������ȼ� 

}

void uart4_init(u32 pclk2,u32 bound)	//����4���� PC10 PC11
{
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//�õ�USARTDIV
	mantissa=temp;				 //�õ���������
	fraction=(temp-mantissa)*16; //�õ�С������	 
  mantissa<<=4;
	mantissa+=fraction; 
	
	RCC->APB2ENR|=1<<4;   //ʹ��PORTC��ʱ��  
	RCC->APB1ENR|=1<<19;  //ʹ�ܴ���ʱ�� 
	
	GPIOC->CRH&=0XFFFF00FF;//IO״̬����
	GPIOC->CRH|=0X00008B00;//IO״̬���� 
	
	RCC->APB1RSTR|=1<<19;   //��λ����
	RCC->APB1RSTR&=~(1<<19);//ֹͣ��λ	   	   
	//����������
 	UART4->BRR=mantissa; // ����������	 
	UART4->CR1|=0X200C;  //1λֹͣ,��У��λ.

	//ʹ�ܽ����ж� 
	UART4->CR1|=1<<5;    //���ջ������ǿ��ж�ʹ��	    	
	MY_NVIC_Init(0,0,UART4_IRQn,2);//��2��������ȼ� 

}

void uart5_init(u32 pclk2,u32 bound)	//����5���� PC12 PD2
{
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//�õ�USARTDIV
	mantissa=temp;				 //�õ���������
	fraction=(temp-mantissa)*16; //�õ�С������	 
  mantissa<<=4;
	mantissa+=fraction; 
	
	RCC->APB2ENR|=1<<4;   //ʹ��PORTC��ʱ�� 
	RCC->APB2ENR|=1<<5;   //ʹ��PORTD��ʱ��  
	
	RCC->APB1ENR|=1<<20;  //ʹ�ܴ���ʱ�� 
	
	GPIOD->CRL&=0XFFFFF0FF;//IO״̬����
	GPIOD->CRL|=0X00000800;//IO״̬���� 
	
	GPIOC->CRH&=0XFFF0FFFF;//IO״̬����
	GPIOC->CRH|=0X000B0000;//IO״̬���� 
	
	RCC->APB1RSTR|=1<<20;   //��λ����
	RCC->APB1RSTR&=~(1<<20);//ֹͣ��λ	   	   
	//����������
 	UART5->BRR=mantissa; // ����������	 
	UART5->CR1|=0X200C;  //1λֹͣ,��У��λ.

	//ʹ�ܽ����ж� 
	UART5->CR1|=1<<5;    //���ջ������ǿ��ж�ʹ��	    	
	MY_NVIC_Init(0,0,UART5_IRQn,2);//��2��������ȼ� 

}




/*���ڽ����ж�*/
void USART1_IRQHandler(void)
{
	u8 res;	
	res=USART1->DR;
	USART_RX_BUF[First_Int]=res;
	First_Int++;                			//����ָ������ƶ�
			if(First_Int > USART_REC_LEN)       		//���������,������ָ��ָ�򻺴���׵�ַ
			{
				First_Int = 0;
			}    

}

/*���ڽ����ж�*/
void USART2_IRQHandler(void)
{
	u8 res;	
	res=USART2->DR;
//		printf("res is %d\n",res);

	USART2_RX_BUF[Uart2RxDataConut++]=res;
	if(Uart2RxDataConut>=USART_LEN)
			Uart2RxDataConut=0;
	
		if(USART2_RX_BUF[1]== Uart2RxDataConut)
			{
				Uart2RxDataConut = 0;
				Rx2Flag = 1;
			}
	
	
}

/*���ڽ����ж�*/
void USART3_IRQHandler(void)
{
	u8 res;	

	res=USART3->DR;
	USART3_RX_BUF[Uart3RxDataConut++]=res;
	if(Uart3RxDataConut>=USART_LEN)
			Uart3RxDataConut=0;
	
		if(USART3_RX_BUF[1]== Uart3RxDataConut)
			{
				Uart3RxDataConut = 0;
				Rx3Flag = 1;

		}
	
}

/*���ڽ����ж�*/
void UART4_IRQHandler(void)
{
		u8 res;	
	res=UART4->DR;
	USART4_RX_BUF[Uart4RxDataConut++]=res;
	if(Uart4RxDataConut>=USART_LEN)
			Uart4RxDataConut=0;
	
		if(USART4_RX_BUF[1]== Uart4RxDataConut)
			{
				Uart4RxDataConut = 0;
				Rx4Flag = 1;
			}
	
}
/*���ڽ����ж�*/
void UART5_IRQHandler(void)
{
	u8 res;	
	res=UART5->DR;
	USART5_RX_BUF[Uart5RxDataConut++]=res;
	if(Uart5RxDataConut>=USART_LEN)
			Uart5RxDataConut=0;
	
		if(USART5_RX_BUF[1]== Uart5RxDataConut)
			{
				Uart5RxDataConut = 0;
				Rx5Flag = 1;
			}
	

}
/*����һ���ַ�*/
void USART_send(u8 ch)
{
	while((USART1->SR&0X40)==0);//�ȴ���һ�δ������ݷ������  
	USART1->DR = (u8) ch; 
}


/*������ܻ���������*/
void CLR_Buf(void)
{
	u16 k;
	for(k=0;k<80;k++)      //��������������
	{
		USART_RX_BUF[k] =0;
	}
    First_Int = 0;              //�����ַ�������ʼ�洢λ��
}

void Uart_Send_Data(USART_TypeDef* USARTx,unsigned char *buf,unsigned char num)
{
	unsigned char i;
	for(i=0;i<num;i++)
	{ 
	 	while((USARTx->SR&0X40)==0);//�ȴ���һ�δ������ݷ������  
		USARTx->DR = (u8) buf[i]; 
	}	
}

unsigned char RxCheckSum(unsigned char *ptr,unsigned char len)
{
	unsigned char i;
	unsigned char checksum;
	checksum = 0;
	for(i=0;i<(len-1);i++)
	{
		   checksum ^= ptr[i];
	}
	checksum = ~checksum;
	if(ptr[len-1] == checksum)
		return 	0;
	else 
		return 	1;
}

void TxCheckSum(unsigned char *ptr,unsigned char len)
{
	unsigned char i;
	unsigned char checksum;
	checksum = 0;
	for(i=0;i<(len-1);i++)
	{
		   checksum ^= ptr[i];
	}
	checksum = ~checksum;
	ptr[len-1] = checksum;
}

unsigned char ReadId(unsigned char *idout,u8 * Uart_Buf,USART_TypeDef* USARTx,u8 *flag)
{
	unsigned char status;
	unsigned char i;
	*flag =0;
	Cmd_Read_Id[5] = 0x01;//������������ʾ
	//Cmd_Read_Id[5] = 0x00;//�رշ�������ʾ
	TxCheckSum(Cmd_Read_Id,Cmd_Read_Id[1]);		//����У���
	Uart_Send_Data(USARTx,Cmd_Read_Id,Cmd_Read_Id[1]);		 //���Ͷ�����ID����
	delay_ms(200);
 	if(*flag == 1)
 	{	
	//	printf("�յ�����\n");
//			for(i=0;i<Uart_Buf[1];i++)
//				printf("%x  ",Uart_Buf[i]);
		*flag = 0;
		status = RxCheckSum(Uart_Buf,Uart_Buf[1]);//�Խ��յ�������У��
		if(status != STATUS_OK)  //�ж�У����Ƿ���ȷ
		{
		//			printf("У��ʧ��\n");

			return STATUS_ERR;
		}
		status = Uart_Buf[4];
		if(status != STATUS_OK)	//�ж��Ƿ���ȷ�Ķ�����
		{
				//	printf("δ������\n");

		 	return STATUS_ERR;
		}
		if((Uart_Buf[0] == 0x01)&&(Uart_Buf[2] == 0xa1))//�ж��Ƿ�Ϊ�����ŷ��ص����ݰ�
		{
			for(i=0;i<4;i++)//��ȡ����ID��6�ֽ�		 
			{
				idout[i] = Uart_Buf[i+7];//������ĵ�5���ֽڿ�ʼΪ���ţ�����Ϊ6�ֽ�
				//printf("%x",Uart_Buf[i]);
			}
			return STATUS_OK;		 //�ɹ�����0
		}
 	} 
	return STATUS_ERR;			//ʧ�ܷ���1
}

