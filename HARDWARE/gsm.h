#include"usart.h"
#include <stm32f10x.h>
#include"delay.h"
#include"string.h"
#include "rc522.h"
#include"sys.h"

#define Success 1U
#define Failure 0U
typedef struct CardInfo
{
	unsigned char state;
	char Cardid[5];
	char umbrella[5];
} Info;


extern char* re;			
extern char* gprs;		
extern char* pdp;			
extern char* getip;		
extern char* connect;	
extern char* tcpsend;
unsigned char Data_Generate(Info* In);
void AT_send(char * str);
u8 A6_init(void);
u8  Tcp_send(Info * card,char * rec,uint16_t tout);
int StringFind(const char *pSrc, const char *pDst);
unsigned int sendCommand(char *Command, char *Response, unsigned long Timeout, unsigned char Retry);
void errorLog(int num);
u8 solve_rec(char * buf);

