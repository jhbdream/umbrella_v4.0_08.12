#include <stm32f10x.h>
#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h" 
#include "key.h"
#include "timer.h"
#include "rc522.h"	
#include "gsm.h"
/*******************************
*����˵����
*1--SS  <----->PB0
*2--SCK <----->PB13
*3--MOSI<----->PB15
*4--MISO<----->PB14
*5--����
*6--GND <----->GND
*7--RST <----->PB1
*8--VCC <----->VCC
************************************/
/*ȫ�ֱ���*/

#define U1_ON 		1500				//ͨ��1		07
#define U1_OFF 		1200
#define U2_ON 		1500					//ͨ��2		08
#define U2_OFF 		1200
#define U3_ON 		1500					//ͨ��3		09
#define U3_OFF		1200
#define U4_ON 		1500				//ͨ��4		11 
#define U4_OFF		1200

extern Info  card;
int time=0;
int main(void)
{		
		
		u8 * cardID;
		int i,ff=0;
		u8 sta;	
		char readbuf[20]={0};
		char cmp[5]={0};              
  		Stm32_Clock_Init(9);					//ϵͳʱ������

		delay_init(72);	
	
		uart_init(72,115200); 				//����1��ʼ��  gsm
		
		uart2_init(36,9600); 			
		uart3_init(36,9600); 				
		uart4_init(36,9600); 				
		uart5_init(36,9600); 				
	
		LED_Init();
		LED2=0;
		PWM_Init(19999,71);
		TIM3_Int_Init(4000,7200-1);		
		InitRc522();									//��ʼ����Ƶ��ģ��
		A6_init();									//gsmģ���ʼ��
		
		Reset_RC522();								//��λrc522ģ��
		cardID=(u8 *)malloc(10);
		LED2=1;
		TIM4->CCR1=U1_OFF;							//��ʼ��ռ�ձ�
		TIM4->CCR2=U2_OFF ;							//��ʼ��ռ�ձ�
		TIM4->CCR3=U3_OFF ;		
		TIM4->CCR4=U4_OFF ;							//��ʼ��ռ�ձ�
		

while(1)		//��ѭ��  ��ѭ��
{		
		LED2=1;
		Reset_RC522();								//��λrc522ģ��
		TIM3->CR1&=~(1<<0);    //�رն�ʱ��3

	while(1)		//Сѭ��  ֻɨ������   ���ɨ�赽��Ƭ����ѭ��
	{
			sta=scan_card(card.Cardid);
		
			delay_ms(100);
			if(sta==1)
			{
			
				LED2=0;
				card.state=0XFF;
				Tcp_send(&card,readbuf,50000);	  //�����������ɨ�赽����Ϣ ��һ���������ɨ�赽�Ŀ�id �ڶ�����ŷ��������ص��ַ���  ������������ʾ�ȴ��������ݳ�ʱ
				solve_rec(readbuf);				//��ȡ���������ص���Ϣ  ��дcard�ṹ�������	
				memset(readbuf,0,20);
				LED2=1;
				break;
			}
		}
		
		switch (card.state)
		{
			case 0x02:                //�û�δ����ɡ
			{
					TIM3->CR1|=0x01;    					//ʹ�ܶ�ʱ��3  ����ɨ�趨ʱ
					time=0;
				while(time<20)
				{				//����ɨ��ʱ��10s  ��ʱֹͣɨ��
					for(i=1;i<5;i++)		//ѭ��ɨ��ɡ��  ���Ҵ��ڵ���ɡ
					{
						//delay_ms(100);
					
						switch(i)
						{
							case 1:							//һ�Ŵӻ�
								{
											 sta=ReadId(card.umbrella,USART2_RX_BUF,USART2,&Rx2Flag);
												if(sta==STATUS_OK)
												{
														card.state=0x01;
														Tcp_send(&card,readbuf,50000);	 
														TIM3->CR1&=~(1<<0);    //�رն�ʱ��3

													TIM4->CCR1=U1_ON;							//��ʼ��ռ�ձ�
														time=50;
														i=100;
														break;  //ȡɡ�ɹ�  ��ǰ����ѭ��
														
												}
												
												break;
								}
						
							case 2:								//���Ŵӻ�
							{
											 sta=ReadId(card.umbrella,USART3_RX_BUF,USART3,&Rx3Flag);
												if(sta==STATUS_OK)
												{
														card.state=0x01;
														Tcp_send(&card,readbuf,50000);									
													TIM3->CR1&=~(1<<0);    //�رն�ʱ��3
												
														TIM4->CCR2=U2_ON;							//��ʼ��ռ�ձ�
														time=50;
														i=100;
														break;  //ȡɡ�ɹ�  ��ǰ����ѭ��
												}
												
												break;							
							}
							
							case 3:								//���Ŵӻ�
							{
											 sta=ReadId(card.umbrella,USART4_RX_BUF,UART4,&Rx4Flag);
												if(sta==STATUS_OK)
												{
														card.state=0x01;
														TIM3->CR1&=~(1<<0);    //�رն�ʱ��3
														Tcp_send(&card,readbuf,50000);	 
														TIM4->CCR3=U3_ON;							//��ʼ��ռ�ձ�
											
														time=50;
														i=100;
														break;  //ȡɡ�ɹ�  ��ǰ����ѭ��
												}
												
												break;							
							}
									case 4:						//�ĺŴӻ�
							{
											 sta=ReadId(card.umbrella,USART5_RX_BUF,UART5,&Rx5Flag);
												if(sta==STATUS_OK)
												{
														card.state=0x01;
														Tcp_send(&card,readbuf,50000);	 
														TIM4->CCR4=U4_ON;							//��ʼ��ռ�ձ�
														TIM3->CR1&=~(1<<0);    //�رն�ʱ��3
														time=50;
														i=100;
														break;  //ȡɡ�ɹ�  ��ǰ����ѭ��
												}
												
												break;							
							}
																					
										}						
						}
				}
				break;
			}
		
		case 0x01:  //��ǰ�û��Ѿ���ɡ
		{
			TIM3->CR1|=0x01;    					//ʹ�ܶ�ʱ��3  ����ɨ�趨ʱ
			time=0;
			while(time<15)		//����ɨ��ʱ��10S ����ʱ��  ��ʱֹͣɨ�� ��ɡʧ��
			{
			for(i=1;i<5;i++)		//ѭ��ɨ��ɡ�� �ȶԸ���ɡ�ܴ��ڵ���ɡ��ID �ͷ��������صĴ��û���ɡ��ID
			{
				
								switch(i)
						{
							case 1:							//һ�Ŵӻ�
								{
									
											 sta=ReadId(cmp,USART2_RX_BUF,USART2,&Rx2Flag);
												if(sta==STATUS_OK)
												{
														if(cmp[0]==card.umbrella[0]&&cmp[1]==card.umbrella[1]&&cmp[2]==card.umbrella[2]&&cmp[3]==card.umbrella[3])
																{
																		//��ɡ���кűȽϳɹ�  ȷ����ɡ  ��ɡ�ɹ�
																		card.state=0X02;
																		Tcp_send(&card,readbuf,50000);	
																		TIM3->CR1&=~(1<<0);    //�رն�ʱ��3
																		TIM4->CCR1=U1_OFF;							//��ʼ��ռ�ձ�
																		time=50;
																		i=100;
															
																break;  //ȡɡ�ɹ�  ��ǰ����ѭ��
																}
														
												}
												
												break;
								}
						
							case 2:								//���Ŵӻ�
							{
											 sta=ReadId(cmp,USART3_RX_BUF,USART3,&Rx3Flag);
												if(sta==STATUS_OK)
												{
														if(cmp[0]==card.umbrella[0]&&cmp[1]==card.umbrella[1]&&cmp[2]==card.umbrella[2]&&cmp[3]==card.umbrella[3])
																{
																		//��ɡ���кűȽϳɹ�  ȷ����ɡ  ��ɡ�ɹ�
																		card.state=0X02;
														TIM3->CR1&=~(1<<0);    //�رն�ʱ��3
																		Tcp_send(&card,readbuf,50000);	
																	
																		TIM4->CCR2=U2_OFF;	
																		time=50;
																			i=100;
																	break;  //ȡɡ�ɹ�  ��ǰ����ѭ��									
																}											

												}
												
												break;							
							}
							
							case 3:								//���Ŵӻ�
							{
											 sta=ReadId(cmp,USART4_RX_BUF,UART4,&Rx4Flag);
												if(sta==STATUS_OK)
												{
														if(cmp[0]==card.umbrella[0]&&cmp[1]==card.umbrella[1]&&cmp[2]==card.umbrella[2]&&cmp[3]==card.umbrella[3])
																{
																		//��ɡ���кűȽϳɹ�  ȷ����ɡ  ��ɡ�ɹ�
																		card.state=0X02;
																		Tcp_send(&card,readbuf,50000);	
																		TIM4->CCR3=U3_OFF;	
																		TIM3->CR1&=~(1<<0);    //�رն�ʱ��3
																		time=50;
																		i=100;
																		break;			//ʶ���û��黹��ɡ  ����ѭ��
																	
																}			
												}
												
												break;							
							}
									case 4:						//�ĺŴӻ�
							{
											 sta=ReadId(cmp,USART5_RX_BUF,UART5,&Rx5Flag);
												if(sta==STATUS_OK)
												{
														if(cmp[0]==card.umbrella[0]&&cmp[1]==card.umbrella[1]&&cmp[2]==card.umbrella[2]&&cmp[3]==card.umbrella[3])
																{
																		//��ɡ���кűȽϳɹ�  ȷ����ɡ  ��ɡ�ɹ�
																		card.state=0X02;
																		Tcp_send(&card,readbuf,50000);
																		TIM3->CR1&=~(1<<0);    //�رն�ʱ��3
																	
																		TIM4->CCR4=U4_OFF;	
																		time=50;
																		i=100;
																		break;			//ʶ���û��黹��ɡ  ����ѭ��
																	
																}																
												}										
												break;																																													
						}					
					}	
				}
		}
				break;
			}
		case 0x03:  //�û�δע��
		{
				break;	
		}
	}

}
	
}
       

