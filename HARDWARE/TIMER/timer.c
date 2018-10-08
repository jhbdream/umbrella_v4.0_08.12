#include "timer.h"
#include "led.h"
#include"rc522.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK Mini STM32开发板
//通用定时器 驱动代码			   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/3/06
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	  
extern int time;

//定时器3中断服务程序	 
void TIM3_IRQHandler(void)
{ 		    		  			    
	if(TIM3->SR&0X0001)//溢出中断
	{
	
		LED2=!LED2;
		time++;		

	}				   
	TIM3->SR&=~(1<<0);//清除中断标志位 	    
}
//通用定时器中断初始化 
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<1;	//TIM3时钟使能    
 	TIM3->ARR=arr;  	//设定计数器自动重装值 
	TIM3->PSC=psc;  	//预分频器设置
	TIM3->DIER|=1<<0;   //允许更新中断				
	TIM3->CR1&=~(1<<0);    //关闭定时器3
  MY_NVIC_Init(1,1,TIM3_IRQn,2);//抢占1，子优先级3，组2									 
}

void PWM_Init(u16 arr,u16 psc)
{		 					 
	//MiniBalance_Motor_Init();  //初始化电机控制所需IO
	RCC->APB2ENR|=1<<3;        //PORTA时钟使能     
	RCC->APB1ENR|=1<<2;       //使能TIM1时钟    
	
	
	GPIOB->CRL&=0X00FFFFFF;    //PORTA8 11复用输出
	GPIOB->CRL|=0XBB000000;    //PORTA8 11复用输出
	GPIOB->CRH&=0XFFFFFF00;    //PORTA8 11复用输出
	GPIOB->CRH|=0X000000BB;    //PORTA8 11复用输出
	
	
	
	TIM4->ARR=arr;             //设定计数器自动重装值 
	TIM4->PSC=psc;             //预分频器不分频
	TIM4->CCMR1|=6<<4;         //CH1 PWM1模式	
	TIM4->CCMR1|=1<<3;         //CH1预装载使能	  
  
	TIM4->CCMR1|=6<<12;         //CH1 PWM1模式	
	TIM4->CCMR1|=1<<11;         //CH1预装载使能

	TIM4->CCMR2|=6<<12;         //CH1 PWM1模式	
	TIM4->CCMR2|=1<<11;         //CH1预装载使能

	TIM4->CCMR2|=6<<4;         //CH1 PWM1模式	
	TIM4->CCMR2|=1<<3;         //CH1预装载使能		
//	TIM4->CCER&=0X00;
	TIM4->CCER|=1<<0;          //CH1输出使能	
	TIM4->CCER|=1<<4;          //CH1输出使能	
	TIM4->CCER|=1<<8;          //CH1输出使能	
	TIM4->CCER|=1<<12;          //CH1输出使能			

	TIM4->CR1|=0x01;          //使能定时器4			

	TIM4->CCR1=1500;							//初始化占空比
	TIM4->CCR2=1500;							//初始化占空比
	TIM4->CCR3=1500;							//初始化占空比
	TIM4->CCR4=1500 ;							//初始化占空比
} 

void Lock(u8 state,u8 pos)
{
	switch (pos)
	{
		case 1:if(state) TIM4->CCR1=1500; else TIM4->CCR1=2000;break;
		
		case 2:if(state) TIM4->CCR2=1500; else TIM4->CCR2=2000;break;;
		
		case 3:if(state) TIM4->CCR3=1500; else TIM4->CCR3=2000;break;;
		
		case 4:if(state) TIM4->CCR4=1500; else TIM4->CCR3=2000;break;;
	
	}


}

//定时器2中断服务程序	 
void TIM2_IRQHandler(void)
{ 		    		  			    
	if(TIM2->SR&0X0001)//溢出中断
	{
	
		LED1=!LED1;

	}				   
	TIM2->SR&=~(1<<0);//清除中断标志位 	    
}
//通用定时器中断初始化 
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
void TIM2_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<0;	//TIM3时钟使能    
 	TIM2->ARR=arr;  	//设定计数器自动重装值 
	TIM2->PSC=psc;  	//预分频器设置
	TIM2->DIER|=1<<0;   //允许更新中断				
	TIM2->CR1&=~(1<<0);    //关闭定时器3
  MY_NVIC_Init(1,1,TIM2_IRQn,2);//抢占1，子优先级3，组2									 
}








