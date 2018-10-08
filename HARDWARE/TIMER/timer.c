#include "timer.h"
#include "led.h"
#include"rc522.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK Mini STM32������
//ͨ�ö�ʱ�� ��������			   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/3/06
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	  
extern int time;

//��ʱ��3�жϷ������	 
void TIM3_IRQHandler(void)
{ 		    		  			    
	if(TIM3->SR&0X0001)//����ж�
	{
	
		LED2=!LED2;
		time++;		

	}				   
	TIM3->SR&=~(1<<0);//����жϱ�־λ 	    
}
//ͨ�ö�ʱ���жϳ�ʼ�� 
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<1;	//TIM3ʱ��ʹ��    
 	TIM3->ARR=arr;  	//�趨�������Զ���װֵ 
	TIM3->PSC=psc;  	//Ԥ��Ƶ������
	TIM3->DIER|=1<<0;   //��������ж�				
	TIM3->CR1&=~(1<<0);    //�رն�ʱ��3
  MY_NVIC_Init(1,1,TIM3_IRQn,2);//��ռ1�������ȼ�3����2									 
}

void PWM_Init(u16 arr,u16 psc)
{		 					 
	//MiniBalance_Motor_Init();  //��ʼ�������������IO
	RCC->APB2ENR|=1<<3;        //PORTAʱ��ʹ��     
	RCC->APB1ENR|=1<<2;       //ʹ��TIM1ʱ��    
	
	
	GPIOB->CRL&=0X00FFFFFF;    //PORTA8 11�������
	GPIOB->CRL|=0XBB000000;    //PORTA8 11�������
	GPIOB->CRH&=0XFFFFFF00;    //PORTA8 11�������
	GPIOB->CRH|=0X000000BB;    //PORTA8 11�������
	
	
	
	TIM4->ARR=arr;             //�趨�������Զ���װֵ 
	TIM4->PSC=psc;             //Ԥ��Ƶ������Ƶ
	TIM4->CCMR1|=6<<4;         //CH1 PWM1ģʽ	
	TIM4->CCMR1|=1<<3;         //CH1Ԥװ��ʹ��	  
  
	TIM4->CCMR1|=6<<12;         //CH1 PWM1ģʽ	
	TIM4->CCMR1|=1<<11;         //CH1Ԥװ��ʹ��

	TIM4->CCMR2|=6<<12;         //CH1 PWM1ģʽ	
	TIM4->CCMR2|=1<<11;         //CH1Ԥװ��ʹ��

	TIM4->CCMR2|=6<<4;         //CH1 PWM1ģʽ	
	TIM4->CCMR2|=1<<3;         //CH1Ԥװ��ʹ��		
//	TIM4->CCER&=0X00;
	TIM4->CCER|=1<<0;          //CH1���ʹ��	
	TIM4->CCER|=1<<4;          //CH1���ʹ��	
	TIM4->CCER|=1<<8;          //CH1���ʹ��	
	TIM4->CCER|=1<<12;          //CH1���ʹ��			

	TIM4->CR1|=0x01;          //ʹ�ܶ�ʱ��4			

	TIM4->CCR1=1500;							//��ʼ��ռ�ձ�
	TIM4->CCR2=1500;							//��ʼ��ռ�ձ�
	TIM4->CCR3=1500;							//��ʼ��ռ�ձ�
	TIM4->CCR4=1500 ;							//��ʼ��ռ�ձ�
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

//��ʱ��2�жϷ������	 
void TIM2_IRQHandler(void)
{ 		    		  			    
	if(TIM2->SR&0X0001)//����ж�
	{
	
		LED1=!LED1;

	}				   
	TIM2->SR&=~(1<<0);//����жϱ�־λ 	    
}
//ͨ�ö�ʱ���жϳ�ʼ�� 
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��3!
void TIM2_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<0;	//TIM3ʱ��ʹ��    
 	TIM2->ARR=arr;  	//�趨�������Զ���װֵ 
	TIM2->PSC=psc;  	//Ԥ��Ƶ������
	TIM2->DIER|=1<<0;   //��������ж�				
	TIM2->CR1&=~(1<<0);    //�رն�ʱ��3
  MY_NVIC_Init(1,1,TIM2_IRQn,2);//��ռ1�������ȼ�3����2									 
}








