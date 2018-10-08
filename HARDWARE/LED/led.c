#include <stm32f10x.h>	   
#include "led.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//LED驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2010/5/27
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved											  
////////////////////////////////////////////////////////////////////////////////// 	   

//初始化PB0 B1 B2为输出口.并使能这两个口的时钟		    
//LED IO初始化
void LED_Init(void)		
{
	RCC->APB2ENR|=1<<3;    //使能PORTB时钟	   	 
	   	 
	GPIOB->CRL&=0XFFFFF000; 
	GPIOB->CRL|=0X00000333;					//PB0  B1 B2 推挽输出   	 
  GPIOB->ODR&=0XFFFFFFF8;      							//PB0  B1 B2输出高
	
}






