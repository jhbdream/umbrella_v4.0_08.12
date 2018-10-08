#include"gsm.h"
////////////////////////////////////////////////////////////////////////////////
//上网连接服务器步骤																													//
//1、AT 返回OK表示串口连接正常																								//
//2、AT+CGATT=1   返回OK表示 附着gprs	返回 	+CTZV:17/03/29,15:35:47,+08 OK		//
//3、AT+CGACT=1,1   返回OK 表示PDP激活网络		
//返回如下数据
//+CIEV: service,  1
//+CIEV: roam, 0
//
//+CREG: 1
//
//OK
//4、AT+CIPSTART="TCP","112.74.103.22",20001  连接到TCP/IP服务器								//
//返回如下数据
//CONNECT OK
//
//OK
//5、AT+CIPSEND=4   返回">" 可以发送四字节的数据  id卡号四字节								//
//	发送完毕后 回复ok
//
//CIPRCV:5,12345   表示从服务器收到数据			
//
//
//发送at指令出错是返回值为  +CME ERROR:58
////////////////////////////////////////////////////////////////////////////////
//char* re			="AT+CREG?";				//查询注册状态
//char* gprs		="AT+CGATT=1";  		//gprs附着
//char* pdp			="AT+CGACT=1,1"; 		//pdp激活
//char* getip		="AT+CIFST";
//char* connect	="AT+CIPSTART=TCP,112.74.103.22,20001";
//char* tcpsend ="AT+CIPSEND=6";
//char* at      ="AT";
//char* cipstatus="AT+CIPSTATUS";
 Info  card;
char Data_Buffer[20];
//char TCPServer[] = "120.76.100.197";		//TCP服务器地址

char TCPServer[] = "120.25.202.56";		//TCP服务器地址
char Port[] = "45576";						//端口
//char TCPServer[] = "120.76.100.197";	
unsigned int count = 0;
unsigned long  Time_Cont = 0;       //定时器计数器
//find string in string, return the first start location or -1 if can not find  
int StringFind(const char *pSrc, const char *pDst)  
{  
    int i, j;  
    for (i=0; pSrc[i]!='\0'; i++)  
    {  
        if(pSrc[i]!=pDst[0])  
            continue;         
        j = 0;  
        while(pDst[j]!='\0' && pSrc[i+j]!='\0')  
        {  
            j++;  
            if(pDst[j]!=pSrc[i+j])  
            break;  
        }  
        if(pDst[j]=='\0')  
            return i;  
    }  
    return -1;  
}  



//发送at指令

void AT_send(char * str)
{
	//strchr(str,5);
	
	while(*str)
	{
		USART_send(*str++);
		
	}
	/*
	USART_send(0X0D);	//结尾回车字符
	delay_ms(5);
	USART_send(0X0D);
*/
}

//初始化函数  连接到指定服务器
u8 A6_init(void)
{

	if (sendCommand("AT\r\n", "OK\r\n", 3000, 10) == Success);
	else errorLog(2);
	delay_ms(10);

	if (sendCommand("AT+CPIN?\r\n", "READY", 1000, 10) == Success);
	else errorLog(3);
	delay_ms(10);

//	if (sendCommand("AT+CREG?\r\n", "0,1", 1000, 10) == Success);
//	else errorLog(4);
//	delay_ms(10);

	if (sendCommand("AT+CSQ\r\n", "OK\r\n", 1000, 10) == Success);
	else errorLog(5);
	delay_ms(10);


}

u8  Tcp_send(Info * card,char * rec,uint16_t tout)
{
		char * ua =USART_RX_BUF;
		uint16_t tc=0,count=0,a;
		u8 * pos;
		char respons[20]={0};
		char send_buf[100] = {0};
		
		if (sendCommand("AT+CGATT?\r\n", "CGATT: 1", 6000, 5) == Success);
		else errorLog(9);
		
		if (sendCommand("AT+CIPSHUT\r\n", "OK", 6000, 5) == Success);
		else errorLog(9);
		
		if (sendCommand("AT+CSTT\r\n", "OK", 6000, 5) == Success);
		else errorLog(9);
		
		if (sendCommand("AT+CIICR\r\n", "OK", 6000, 5) == Success);
		else errorLog(9);
			if (sendCommand("AT+CIFSR\r\n", "", 6000, 5) == Success);
		else errorLog(9);

		
		
		memset(send_buf, 0, 100);    //清空
		memset(respons, 0, 20);    //清空
		strcpy(send_buf, "AT+CIPSTART=\"TCP\",\"");
		strcat(send_buf, TCPServer);
		strcat(send_buf, "\",");
		strcat(send_buf, Port);
		strcat(send_buf, "\r\n");
		if (sendCommand(send_buf, "CONNECT OK", 100000, 5) == Success);
		else errorLog(8);
		delay_ms(20);
		//发送数据
		if (sendCommand("AT+CIPSEND\r\n", ">", 5000, 5) == Success);
		else errorLog(9);
		
		count=Data_Generate(card);		//生成发送数据包  返回数据包大小
		for(a=0;a<count;a++)
		{
			USART_send(Data_Buffer[a]);	
		}
		delay_ms(200);
		memset(send_buf, 0, 100);    //清空
		CLR_Buf();
		sprintf(send_buf,"%c\r\n",0x1a);
		AT_send(send_buf);
		memset(send_buf, 0, 100);    //清空
		sprintf(send_buf,"%c",0x55);	
	if(card->state==0xff)
	{
		while (tc < tout)
		{
				delay_ms(500);
		
				tc+=10;

			if(strstr(USART_RX_BUF,send_buf) != NULL)
			{
						strcpy(rec,USART_RX_BUF);	
						CLR_Buf();
								
						if (sendCommand("AT+CIPCLOSE\r\n", "OK\r\n", 3000, 10) == Success);
						else errorLog(12);
							
						return Success;
							
					}
		}
	}
	
		Time_Cont =0;
		CLR_Buf();
		if (sendCommand("AT+CIPCLOSE\r\n", "OK\r\n", 3000, 10) == Success);
		else errorLog(12);
		return Success;
}
	

unsigned int sendCommand(char *Command, char *Response, unsigned long Timeout,unsigned char Retry)
{
	unsigned char n;
	CLR_Buf();
	for(n = 0;n < Retry; n++)
	{
		AT_send(Command);
		
//		printf("\r\n***************send****************\r\n");
//		printf(Command);
		
		Time_Cont=0;
		while (Time_Cont < Timeout)
		{
				delay_ms(200);
		
				Time_Cont+=100;
				
			if(strstr(USART_RX_BUF, Response) != NULL)
					{

							CLR_Buf();
							return Success;
					}
		}
	
		Time_Cont =0;
	}

	CLR_Buf();
	return Failure;
	
}


void errorLog(int num)
{
	while(1)
	{
		if(sendCommand("AT\r\n","OK", 100, 10) ==Success)
		{
			Sys_Soft_Reset();
		}
		
		delay_ms(200);
		
	}
}


u8 solve_rec(char * buf)
{
		u8 i=0;
	for(i=0;i<30;i++)
	{
					
		if(buf[i]==0x55)
					{
						if((buf[i+2]==0x55)||(buf[i+1]==0x01&&buf[i+6]==0x55))
							
							switch(buf[i+1])
							{
								case 0x02:	
								{
									card.state=0X02;
									card.umbrella[0]=0x00; 
								//	printf("收到 0x00,	未借伞 	 查询存在的雨伞并返回卡的ID 解锁舵机\n");
									memset(buf,0,sizeof(buf));	
									break;
								}								
								
								case 0x01:
								{
									card.state=0X01;
									card.umbrella[0]=buf[i+2]; 
									card.umbrella[1]=buf[i+3]; 
									card.umbrella[2]=buf[i+4]; 
									card.umbrella[3]=buf[i+5]; 
								//	printf("%x %x %x %x\n",card.umbrella[0],card.umbrella[1],card.umbrella[2],card.umbrella[3]);  
						//		printf("收到 0x01,	已经借伞 服务器同时返回了雨伞ID  返回舵机  检查rfid 扫描ID 比对  确认是否还伞\n");  
									memset(buf,0,sizeof(buf));	
									break;
								}
								
								case 0x03: card.state=0X03;memset(buf,0,sizeof(buf));	break;
								
								default :	break;
							
			
								}
							
						i=41;
						memset(buf,0,sizeof(buf));				
					}
		}

}

unsigned char Data_Generate(Info* In)			//打包向上位机发送的数据  
{
	Data_Buffer[0]=0x55;											//帧头 0X55
	Data_Buffer[1]=In->state;	
	switch (In->state)
	{
		case 0x02:
		{
			
			Data_Buffer[2]=In->Cardid[0];
			Data_Buffer[3]=In->Cardid[1];
			Data_Buffer[4]=In->Cardid[2];
			Data_Buffer[5]=In->Cardid[3];
			Data_Buffer[6]=0x55;
			return 7;
			break;
		}
		
		case 0x01:
		{
			Data_Buffer[2]=In->Cardid[0];
			Data_Buffer[3]=In->Cardid[1];
			Data_Buffer[4]=In->Cardid[2];
			Data_Buffer[5]=In->Cardid[3];
			Data_Buffer[6]=In->umbrella[0];
			Data_Buffer[7]=In->umbrella[1];
			Data_Buffer[8]=In->umbrella[2];
			Data_Buffer[9]=In->umbrella[3];
			Data_Buffer[10]=0x55;
			return 11;
			break;

		}
		
		
		case 0xff:
		{
		
			Data_Buffer[2]=In->Cardid[0];
			Data_Buffer[3]=In->Cardid[1];
			Data_Buffer[4]=In->Cardid[2];
			Data_Buffer[5]=In->Cardid[3];
			Data_Buffer[6]=0x55;
			return 7;
		break;
		}
	
		default :return 0; ;
	
	
	}

	
	

}




