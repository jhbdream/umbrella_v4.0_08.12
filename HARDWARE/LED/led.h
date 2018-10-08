#ifndef __LED_H
#define __LED_H	 
#include "sys.h"
//Mini STM32开发板
//LED驱动代码			 
//正点原子@ALIENTEK
//2010/5/27

//LED端口定义
#define LED0 PBout(0)
#define LED1 PBout(1)
#define LED2 PBout(1)		//红灯 工作状态指示 
void LED_Init(void);//初始化		 				    
#endif

















