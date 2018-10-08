#include "usart.h"	  
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif
#define STATUS_OK			0x00
#define STATUS_ERR    0x01
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef’ d in stdio.h. */ 
FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定向fputc函数
//printf的输出，指向fputc，由fputc输出到串口
//这里使用串口1(USART1)输出printf信息
int fputc(int ch, FILE *f)
{      
	while((USART2->SR&0X40)==0);//等待上一次串口数据发送完成  
	USART2->DR = (u8) ch;      	//写DR,串口1将发送数据
	return ch;
}
#endif 
//end
//////////////////////////////////////////////////////////////////
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
u8 USART2_RX_BUF[USART_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
u8 USART3_RX_BUF[USART_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
u8 USART4_RX_BUF[USART_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
u8 USART5_RX_BUF[USART_LEN];     //接收缓冲,最大USART_REC_LEN个字节.

u8 Uart2RxDataConut=0;
u8 Uart3RxDataConut=0;
u8 Uart4RxDataConut=0;
u8 Uart5RxDataConut=0;

u8 Rx2Flag = 0;
u8 Rx3Flag = 0;
u8 Rx4Flag = 0;
u8 Rx5Flag = 0;

unsigned char Cmd_Read_Id[8] = {0x01,0x08,0xa1,0x20,0x00,0x00,0x00,0x00};

//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  
u8 First_Int =0; //当前缓存区长度

//初始化IO 串口1
//pclk2:PCLK2时钟频率(Mhz)
//bound:波特率 
void uart_init(u32 pclk2,u32 bound)
{  	 
	
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分	 
    mantissa<<=4;
	mantissa+=fraction; 
	RCC->APB2ENR|=1<<2;   //使能PORTA口时钟  
	RCC->APB2ENR|=1<<14;  //使能串口时钟 
	GPIOA->CRH&=0XFFFFF00F;//IO状态设置
	GPIOA->CRH|=0X000008B0;//IO状态设置 
	RCC->APB2RSTR|=1<<14;   //复位串口1
	RCC->APB2RSTR&=~(1<<14);//停止复位	   	   
	//波特率设置
 	USART1->BRR=mantissa; // 波特率设置	 
	USART1->CR1|=0X200C;  //1位停止,无校验位.

	//使能接收中断 
	USART1->CR1|=1<<5;    //接收缓冲区非空中断使能	    	
	MY_NVIC_Init(0,0,USART1_IRQn,2);//组2，最低优先级 
}


void uart2_init(u32 pclk2,u32 bound)
{
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分	 
    mantissa<<=4;
	mantissa+=fraction; 
	RCC->APB2ENR|=1<<2;   //使能PORTA口时钟  
	RCC->APB1ENR|=1<<17;  //使能串口时钟 
	GPIOA->CRL&=0XFFFF00FF;//IO状态设置
	GPIOA->CRL|=0X00008B00;//IO状态设置 
	RCC->APB1RSTR|=1<<17;   //复位串口1
	RCC->APB1RSTR&=~(1<<17);//停止复位	   	   
	//波特率设置
 	USART2->BRR=mantissa; // 波特率设置	 
	USART2->CR1|=0X200C;  //1位停止,无校验位.

	//使能接收中断 
	USART2->CR1|=1<<5;    //接收缓冲区非空中断使能	    	
	MY_NVIC_Init(0,0,USART2_IRQn,2);//组2，最低优先级 

}

void uart3_init(u32 pclk2,u32 bound)	//串口3配置 PD8 PD9
{
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分	 
  mantissa<<=4;
	mantissa+=fraction; 
		
	RCC->APB2ENR|=1<<0;   //使能AFIO时钟
	RCC->APB2ENR|=1<<5;   //使能PORTD口时钟  
	RCC->APB1ENR|=1<<18;  //使能串口时钟 
	
	GPIOD->CRH&=0XFFFFFF00;//IO状态设置
	GPIOD->CRH|=0X0000008B;//IO状态设置 
	AFIO->MAPR|=3<<4;		//串口3完全重映射
	
	RCC->APB1RSTR|=1<<18;   //复位串口
	RCC->APB1RSTR&=~(1<<18);//停止复位	   	   
	//波特率设置
 	USART3->BRR=mantissa; // 波特率设置	 
	USART3->CR1|=0X200C;  //1位停止,无校验位.

	//使能接收中断 
	USART3->CR1|=1<<5;    //接收缓冲区非空中断使能	    	
	MY_NVIC_Init(0,0,USART3_IRQn,2);//组2，最低优先级 

}

void uart4_init(u32 pclk2,u32 bound)	//串口4配置 PC10 PC11
{
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分	 
  mantissa<<=4;
	mantissa+=fraction; 
	
	RCC->APB2ENR|=1<<4;   //使能PORTC口时钟  
	RCC->APB1ENR|=1<<19;  //使能串口时钟 
	
	GPIOC->CRH&=0XFFFF00FF;//IO状态设置
	GPIOC->CRH|=0X00008B00;//IO状态设置 
	
	RCC->APB1RSTR|=1<<19;   //复位串口
	RCC->APB1RSTR&=~(1<<19);//停止复位	   	   
	//波特率设置
 	UART4->BRR=mantissa; // 波特率设置	 
	UART4->CR1|=0X200C;  //1位停止,无校验位.

	//使能接收中断 
	UART4->CR1|=1<<5;    //接收缓冲区非空中断使能	    	
	MY_NVIC_Init(0,0,UART4_IRQn,2);//组2，最低优先级 

}

void uart5_init(u32 pclk2,u32 bound)	//串口5配置 PC12 PD2
{
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分	 
  mantissa<<=4;
	mantissa+=fraction; 
	
	RCC->APB2ENR|=1<<4;   //使能PORTC口时钟 
	RCC->APB2ENR|=1<<5;   //使能PORTD口时钟  
	
	RCC->APB1ENR|=1<<20;  //使能串口时钟 
	
	GPIOD->CRL&=0XFFFFF0FF;//IO状态设置
	GPIOD->CRL|=0X00000800;//IO状态设置 
	
	GPIOC->CRH&=0XFFF0FFFF;//IO状态设置
	GPIOC->CRH|=0X000B0000;//IO状态设置 
	
	RCC->APB1RSTR|=1<<20;   //复位串口
	RCC->APB1RSTR&=~(1<<20);//停止复位	   	   
	//波特率设置
 	UART5->BRR=mantissa; // 波特率设置	 
	UART5->CR1|=0X200C;  //1位停止,无校验位.

	//使能接收中断 
	UART5->CR1|=1<<5;    //接收缓冲区非空中断使能	    	
	MY_NVIC_Init(0,0,UART5_IRQn,2);//组2，最低优先级 

}




/*串口接受中断*/
void USART1_IRQHandler(void)
{
	u8 res;	
	res=USART1->DR;
	USART_RX_BUF[First_Int]=res;
	First_Int++;                			//缓存指针向后移动
			if(First_Int > USART_REC_LEN)       		//如果缓存满,将缓存指针指向缓存的首地址
			{
				First_Int = 0;
			}    

}

/*串口接受中断*/
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

/*串口接受中断*/
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

/*串口接受中断*/
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
/*串口接受中断*/
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
/*发送一个字符*/
void USART_send(u8 ch)
{
	while((USART1->SR&0X40)==0);//等待上一次串口数据发送完成  
	USART1->DR = (u8) ch; 
}


/*清除接受缓冲区数据*/
void CLR_Buf(void)
{
	u16 k;
	for(k=0;k<80;k++)      //将缓存内容清零
	{
		USART_RX_BUF[k] =0;
	}
    First_Int = 0;              //接收字符串的起始存储位置
}

void Uart_Send_Data(USART_TypeDef* USARTx,unsigned char *buf,unsigned char num)
{
	unsigned char i;
	for(i=0;i<num;i++)
	{ 
	 	while((USARTx->SR&0X40)==0);//等待上一次串口数据发送完成  
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
	Cmd_Read_Id[5] = 0x01;//开启蜂鸣器提示
	//Cmd_Read_Id[5] = 0x00;//关闭蜂鸣器提示
	TxCheckSum(Cmd_Read_Id,Cmd_Read_Id[1]);		//计算校验和
	Uart_Send_Data(USARTx,Cmd_Read_Id,Cmd_Read_Id[1]);		 //发送读卡号ID命令
	delay_ms(200);
 	if(*flag == 1)
 	{	
	//	printf("收到数据\n");
//			for(i=0;i<Uart_Buf[1];i++)
//				printf("%x  ",Uart_Buf[i]);
		*flag = 0;
		status = RxCheckSum(Uart_Buf,Uart_Buf[1]);//对接收到的数据校验
		if(status != STATUS_OK)  //判断校验和是否正确
		{
		//			printf("校验失败\n");

			return STATUS_ERR;
		}
		status = Uart_Buf[4];
		if(status != STATUS_OK)	//判断是否正确的读到卡
		{
				//	printf("未读到卡\n");

		 	return STATUS_ERR;
		}
		if((Uart_Buf[0] == 0x01)&&(Uart_Buf[2] == 0xa1))//判断是否为读卡号返回的数据包
		{
			for(i=0;i<4;i++)//获取卡号ID，6字节		 
			{
				idout[i] = Uart_Buf[i+7];//从数组的第5个字节开始为卡号，长度为6字节
				//printf("%x",Uart_Buf[i]);
			}
			return STATUS_OK;		 //成功返回0
		}
 	} 
	return STATUS_ERR;			//失败返回1
}

