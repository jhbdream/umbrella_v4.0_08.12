#include "sim900a.h"
#include "usart.h"		
#include "delay.h"	 
#include "string.h" 
#include "key.h"
#include "usart2.h" 
#include "math.h"
#include "stdio.h"
//********************************************************************************
//无
//////////////////////////////////////////////////////////////////////////////////	
u8 SIM900_CSQ[3];
u8 dtbuf[50];   								//打印缓存器	
u8 Flag_Rec_Message=0;
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//usmart支持部分 
//将收到的AT指令应答数据返回给电脑串口
//mode:0,不清零USART2_RX_STA;
//     1,清零USART2_RX_STA;
void sim_at_response(u8 mode)
{
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
		printf("%s",USART2_RX_BUF);	//发送到串口
		if(mode)USART2_RX_STA=0;		
	} 
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//ATK-SIM900A 各项测试(拨号测试、短信测试、GPRS测试)共用代码
//sim900a发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//    其他,期待应答结果的位置(str的位置)
u8* sim900a_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//向sim900a发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 sim900a_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;USART2_RX_REC_ATCOMMAD=1;
	if((u32)cmd<=0XFF)
	{
		while(DMA1_Channel7->CNDTR!=0);	//等待通道7传输完成   
		USART2->DR=(u32)cmd;
	}else u2_printf("%s\r\n",cmd);//发送命令
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//接收到期待的应答结果
			{
				if(sim900a_check_cmd(ack))break;//得到有效数据 
				USART2_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	USART2_RX_STA=0;USART2_RX_REC_ATCOMMAD=0;
	return res;
} 

u8 sim900a_work_test(void)
{
	if(sim900a_send_cmd((u8 *)"AT",(u8 *)"OK",100))
	{
		if(sim900a_send_cmd((u8 *)"AT",(u8 *)"OK",100))return SIM_COMMUNTION_ERR;	//通信不上
	}		
	if(sim900a_send_cmd((u8 *)"AT+CPIN?",(u8 *)"READY",400))return SIM_CPIN_ERR;	//没有SIM卡
	if(sim900a_send_cmd((u8 *)"AT+CREG?",(u8 *)"0,1",400))
	{
		if(strstr((const char*)USART2_RX_BUF,"0,5")==NULL)
		{
			 if(!sim900a_send_cmd((u8 *)"AT+CSQ",(u8 *)"OK",200))	
			 {
					memcpy(SIM900_CSQ,USART2_RX_BUF+15,2);
			 }
			 return SIM_CREG_FAIL;	//等待附着到网络
		}
	}	
	return SIM_OK;
}
u8 GSM_Dect(void)
{
	u8 res;
	res=sim900a_work_test();	
	switch(res)
	{
		case SIM_OK:
			UART3SendString("GSM模块自检成功\r\n",strlen("GSM模块自检成功\r\n"));
			break;
		case SIM_COMMUNTION_ERR:
			UART3SendString("与GSM模块未通讯成功，请等待\r\n",strlen("与GSM模块未通讯成功，请等待\r\n"));
			break;
		case SIM_CPIN_ERR:
			UART3SendString("没检测到SIM卡\r\n",strlen("没检测到SIM卡\r\n"));	
			break;
		case SIM_CREG_FAIL:
			UART3SendString("注册网络中。。。\r\n",strlen("注册网络中。。。\r\n"));	
			UART3SendString("当前信号值：",strlen("当前信号值："));	UART3SendString(SIM900_CSQ,2);UART3SendString("\r\n",2);
			break;		
		default:
			break;
	}
	return res;
}
u8 SIM900A_CONNECT_SERVER(u8 *IP_ADD,u8 *COM)
{		
		if(sim900a_send_cmd((u8 *)"AT+CGATT?",(u8 *)": 1",100))	 return 1;
		//if(sim900a_send_cmd((u8 *)"AT+CIPHEAD=1",(u8 *)"OK",100))	 return 7;
	  if(sim900a_send_cmd((u8 *)"AT+CIPSHUT",(u8 *)"OK",500))	return 2;
		if(sim900a_send_cmd((u8 *)"AT+CSTT",(u8 *)"OK",200))	return 3;
		if(sim900a_send_cmd((u8 *)"AT+CIICR",(u8 *)"OK",600))	return 4;
		if(!sim900a_send_cmd((u8 *)"AT+CIFSR",(u8 *)"ERROR",200))	return 5;		
		sprintf((char*)dtbuf,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"",IP_ADD,COM);
	  if(sim900a_send_cmd((u8 *)dtbuf,(u8 *)"CONNECT OK",200))	return 6;		
	  return 0;
}	
u8 SIM900A_CONNECT_SERVER_SEND_INFOR(u8 *IP_ADD,u8 *COM)
{
	u8 res;	
	res=SIM900A_CONNECT_SERVER(IP_ADD,COM);
	switch(res)
	{
		case 0:
			UART3SendString("服务器连接成功\r\n",strlen("服务器连接成功\r\n"));	
			break;
		case 1:
			UART3SendString("等待GSM模块附着网络\r\n",strlen("等待GSM模块附着网络\r\n"));	
			break;
		case 2:
			UART3SendString("场景关闭失败\r\n",strlen("场景关闭失败\r\n"));	
			break;
		case 3:
			UART3SendString("CSTT失败\r\n",strlen("CSTT失败\r\n"));	
			break;
		case 4:
			UART3SendString("CIICR失败\r\n",strlen("CIICR失败\r\n"));	
			break;
		case 5:
			UART3SendString("CIFSR失败\r\n",strlen("CIFSR失败\r\n"));	
			break;
		case 6:
			UART3SendString("连接服务器失败\r\n",strlen("连接服务器失败\r\n"));	
			break;
		default:
			break;
	}
	return res;
}
u8 SIM900A_GPRS_SEND_DATA(u8 *temp_data)
{		
	 //UART3SendString("启动数据发送，数据内容：",strlen("启动数据发送，数据内容："));	
	 if(sim900a_send_cmd("AT+CIPSEND",">",100))	 return 1;
	 //UART3SendString((u8*)temp_data,strlen((u8*)temp_data));	UART3SendString("\r\n",2);
	 if(sim900a_send_cmd(temp_data,NULL,0))	return 2;
	 if(sim900a_send_cmd((u8 *)0x1a,(u8 *)"SEND OK",1500))	return 3;		
	 //UART3SendString("数据发送成功",strlen("数据发送成功"));	UART3SendString("\r\n",2);
	 return 0;
}	
