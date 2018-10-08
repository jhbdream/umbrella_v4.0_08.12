#include "sys.h"
#include "rc522.h"

unsigned char CT[2];//卡类型
unsigned char SN[4]; //卡号
unsigned char status;
int spi_slave=0;  //SPI从设备选择 根据此变量选择片选信号PBX  

void delay_ns(u32 ns)
{
  u32 i;
  for(i=0;i<ns;i++)
  {
    __nop();
    __nop();
    __nop();
  }
}

u8 SPIWriteByte(u8 Byte)
{
	while((SPI2->SR&0X02)==0);		//等待发送区空	  
	SPI2->DR=Byte;	 	//发送一个byte   
	while((SPI2->SR&0X01)==0);//等待接收完一个byte  
	return SPI2->DR;          	     //返回收到的数据			
}
void SPI2_Init(void)	
{ 
	//配置SPI2管脚

	RCC->APB2ENR|=1<<0;
	RCC->APB2ENR|=1<<3;
	RCC->APB2ENR|=1<<5;
	RCC->APB2ENR|=1<<6;
	
	GPIOB->CRH&=0X000F00FF;		
	GPIOB->CRH|=0XB8B03300;
	
	GPIOD->CRH&=0XFFFF0FFF;
	GPIOD->CRH|=0X00003000;

	GPIOB->CRL&=0XFFFFF000;		 //PB.1 复位
	GPIOB->CRL|=0X00000333;		//PB.0 片选 推挽输出

	GPIOE->CRL&=0XFFFFFFF0;		//初始化PE13 PE15 PE14 PE11片选引脚
	GPIOE->CRL|=0X00000003;			//初始化PE0 指示
	
	GPIOE->CRH&=0XFFFFFFF0;		//初始化PE13 PE15 PE14 PE11片选引脚
	GPIOE->CRH|=0X00000003;

	

 PBout(10) =1;
 
	RCC->APB1ENR|=1<<14;

	SPI2->CR1|=0<<10;//全双工模式	
	SPI2->CR1&=0<<9; //软件nss管理
	SPI2->CR1|=1<<8;  
	
	SPI2->CR1|=1<<2; //SPI主机
	SPI2->CR1|=0<<11;//8bit数据格式	
	SPI2->CR1&=~0x02; //空闲模式下SCK为0 CPOL=0
	SPI2->CR1&=~0x01; //数据采样从第一个时间边沿开始,CPHA=0  
	SPI2->CR1|=7<<3; //Fsck=Fcpu/4
	SPI2->CR1|=0<<7; //MSBfirst   
	SPI2->CR1|=1<<6; //SPI设备使能 
}

void InitRc522(void)
{
  SPI2_Init();
  PcdReset();
  PcdAntennaOff();  
  PcdAntennaOn();
  M500PcdConfigISOType( 'A' );
}

void Reset_RC522(void)
{
	PcdReset();
  PcdAntennaOff();  
  PcdAntennaOn();
}                         
/////////////////////////////////////////////////////////////////////
//功    能：寻卡
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡
//                0x26 = 寻未进入休眠状态的卡
//          pTagType[OUT]：卡片类型代码
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////

char PcdRequest(u8   req_code,u8 *pTagType)
{
	char   status;  
	u8   unLen;
	u8   ucComMF522Buf[MAXRLEN]; 

	ClearBitMask(Status2Reg,0x08);
	WriteRawRC(BitFramingReg,0x07);
	SetBitMask(TxControlReg,0x03);
 
	ucComMF522Buf[0] = req_code;

	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);

	if ((status == MI_OK) && (unLen == 0x10))
	{    
		*pTagType     = ucComMF522Buf[0];
		*(pTagType+1) = ucComMF522Buf[1];
	}
	else
	{   status = MI_ERR;   }
   
	return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：防冲撞
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////  
char PcdAnticoll(u8 *pSnr)
{
    char   status;
    u8   i,snr_check=0;
    u8   unLen;
    u8   ucComMF522Buf[MAXRLEN]; 
    

    ClearBitMask(Status2Reg,0x08);
    WriteRawRC(BitFramingReg,0x00);
    ClearBitMask(CollReg,0x80);
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status == MI_OK)
    {
    	 for (i=0; i<4; i++)
         {   
             *(pSnr+i)  = ucComMF522Buf[i];
             snr_check ^= ucComMF522Buf[i];
         }
         if (snr_check != ucComMF522Buf[i])
         {   status = MI_ERR;    }
    }
    
    SetBitMask(CollReg,0x80);
    return status;
}


/////////////////////////////////////////////////////////////////////
//功    能：选定卡片
//参数说明: pSnr[IN]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdSelect(u8 *pSnr)
{
    char   status;
    u8   i;
    u8   unLen;
    u8   ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    ClearBitMask(Status2Reg,0x08);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   status = MI_OK;  }
    else
    {   status = MI_ERR;    }

    return status;
}


/////////////////////////////////////////////////////////////////////
//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdHalt(void)
{
    u8   status;
    u8   unLen;
    u8   ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//用MF522计算CRC16函数
/////////////////////////////////////////////////////////////////////
void CalulateCRC(u8 *pIn ,u8   len,u8 *pOut )
{
    u8   i,n;
    ClearBitMask(DivIrqReg,0x04);
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    for (i=0; i<len; i++)
    {   WriteRawRC(FIFODataReg, *(pIn +i));   }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOut [0] = ReadRawRC(CRCResultRegL);
    pOut [1] = ReadRawRC(CRCResultRegM);
}

/////////////////////////////////////////////////////////////////////
//功    能：复位RC522
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdReset(void)
{
	
		SET_RC522RST;
    delay_ns(10);
	
		CLR_RC522RST;
    delay_ns(10);
	
		SET_RC522RST;
    delay_ns(10);
	
    WriteRawRC(CommandReg,PCD_RESETPHASE);
		WriteRawRC(CommandReg,PCD_RESETPHASE);
    delay_ns(10);
    
    WriteRawRC(ModeReg,0x3D);            //和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(TReloadRegL,30);           
    WriteRawRC(TReloadRegH,0);
    WriteRawRC(TModeReg,0x8D);
    WriteRawRC(TPrescalerReg,0x3E);
	
		WriteRawRC(TxAutoReg,0x40);//必须要
   
    return MI_OK;
}

//////////////////////////////////////////////////////////////////////
//设置RC632的工作方式 
//////////////////////////////////////////////////////////////////////
char M500PcdConfigISOType(u8   type)
{
   if (type == 'A')                     //ISO14443_A
   { 
       ClearBitMask(Status2Reg,0x08);
       WriteRawRC(ModeReg,0x3D);//3F
       WriteRawRC(RxSelReg,0x86);//84
       WriteRawRC(RFCfgReg,0x7F);   //4F
   	   WriteRawRC(TReloadRegL,30);//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
	   WriteRawRC(TReloadRegH,0);
       WriteRawRC(TModeReg,0x8D);
	   WriteRawRC(TPrescalerReg,0x3E);
	   delay_ns(1000);
       PcdAntennaOn();
   }
   else{ return 1; }
   
   return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//功    能：读RC522寄存器
//参数说明：Address[IN]:寄存器地址
//返    回：读出的值
/////////////////////////////////////////////////////////////////////
u8 ReadRawRC(u8 Address)
{
    u8   ucAddr;
    u8   ucResult=0;
	
		switch(spi_slave)
		{
			case 0:  
			{
				SPI_CS0=0;	
				
				//拉低片选后，保持100ns。
				//参见RC522数据手册P90，Tsclkl参数
				//by Chai
				delay_ns(1);
				
				ucAddr = ((Address<<1)&0x7E)|0x80;
				SPIWriteByte(ucAddr);
				ucResult=SPIReadByte();
				SPI_CS0=1;
				break;
			}
			case 1:  
			{
				SPI_CS1=0;		
				ucAddr = ((Address<<1)&0x7E)|0x80;
				SPIWriteByte(ucAddr);
				ucResult=SPIReadByte();
				SPI_CS1=1;
				break;
			}
		
				case 2:  
			{
				SPI_CS2=0;
				ucAddr = ((Address<<1)&0x7E)|0x80;
				SPIWriteByte(ucAddr);
				ucResult=SPIReadByte();
				SPI_CS2=1;
				break;
			}
			
					case 3:  
			{
				SPI_CS3=0;
				ucAddr = ((Address<<1)&0x7E)|0x80;
				SPIWriteByte(ucAddr);
				ucResult=SPIReadByte();
				SPI_CS3=1;
				break;
			}
			
				case 4:  
			{
				SPI_CS4=0;
				ucAddr = ((Address<<1)&0x7E)|0x80;
				SPIWriteByte(ucAddr);
				ucResult=SPIReadByte();
				SPI_CS4=1;
				break;
			}
		
		}
//		CLR_SPI_CS(spi_slave);
//    ucAddr = ((Address<<1)&0x7E)|0x80;
//	
//		SPIWriteByte(ucAddr);
//		ucResult=SPIReadByte();
//		SET_SPI_CS(spi_slave);
		return ucResult;
}

/////////////////////////////////////////////////////////////////////
//功    能：写RC522寄存器
//参数说明：Address[IN]:寄存器地址
//          value[IN]:写入的值
/////////////////////////////////////////////////////////////////////
void WriteRawRC(u8   Address, u8   value)
{  
    u8   ucAddr;

	switch(spi_slave)
	{
		case 0: 
		{
			SPI_CS0=0;
			
			//拉低片选后，保持100ns。
			//参见RC522数据手册P90，Tsclkl参数
			//by Chai
			delay_ns(1);
			
			ucAddr = ((Address<<1)&0x7E);
			SPIWriteByte(ucAddr);
			SPIWriteByte(value);
			SPI_CS0=1;
			break;
		}
		
		case 1: 
		{
			SPI_CS1=0;
			ucAddr = ((Address<<1)&0x7E);
			SPIWriteByte(ucAddr);
			SPIWriteByte(value);
			SPI_CS1=1;
			break;
		}
	
		case 2: 
		{
			SPI_CS2=0;
			ucAddr = ((Address<<1)&0x7E);
			SPIWriteByte(ucAddr);
			SPIWriteByte(value);
			SPI_CS2=1;
			break;
		}
		case 3: 
		{
			SPI_CS3=0;
			ucAddr = ((Address<<1)&0x7E);
			SPIWriteByte(ucAddr);
			SPIWriteByte(value);
			SPI_CS3=1;
			break;
		}
		case 4: 
		{
			SPI_CS4=0;
			ucAddr = ((Address<<1)&0x7E);
			SPIWriteByte(ucAddr);
			SPIWriteByte(value);
			SPI_CS4=1;
			break;
		}
				
	
	}
//	CLR_SPI_CS(spi_slave);
//  ucAddr = ((Address<<1)&0x7E);

//	SPIWriteByte(ucAddr);
//	SPIWriteByte(value);
//	SET_SPI_CS(spi_slave);

}

/////////////////////////////////////////////////////////////////////
//功    能：置RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:置位值
/////////////////////////////////////////////////////////////////////
void SetBitMask(u8   reg,u8   mask)  
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}

/////////////////////////////////////////////////////////////////////
//功    能：清RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:清位值
/////////////////////////////////////////////////////////////////////
void ClearBitMask(u8   reg,u8   mask)  
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
} 

/////////////////////////////////////////////////////////////////////
//功    能：通过RC522和ISO14443卡通讯
//参数说明：Command[IN]:RC522命令字
//          pIn [IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOut [OUT]:接收到的卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
/////////////////////////////////////////////////////////////////////
char PcdComMF522(u8   Command, 
                 u8 *pIn , 
                 u8   InLenByte,
                 u8 *pOut , 
                 u8 *pOutLenBit)
{
    char   status = MI_ERR;
    u8   irqEn   = 0x00;
    u8   waitFor = 0x00;
    u8   lastBits;
    u8   n;
    u16   i;
    switch (Command)
    {
        case PCD_AUTHENT:
			irqEn   = 0x12;
			waitFor = 0x10;
			break;
		case PCD_TRANSCEIVE:
			irqEn   = 0x77;
			waitFor = 0x30;
			break;
		default:
			break;
    }
   
    WriteRawRC(ComIEnReg,irqEn|0x80);
    ClearBitMask(ComIrqReg,0x80);	//清所有中断位
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);	 	//清FIFO缓存
    
    for (i=0; i<InLenByte; i++)
    {   WriteRawRC(FIFODataReg, pIn [i]);    }
    WriteRawRC(CommandReg, Command);	  
//   	 n = ReadRawRC(CommandReg);
    
    if (Command == PCD_TRANSCEIVE)
    {    SetBitMask(BitFramingReg,0x80);  }	 //开始传送
    										 
    //i = 600;//根据时钟频率调整，操作M1卡最大等待时间25ms
	//i = 100000;
		i = 2000;
    do 
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));
    ClearBitMask(BitFramingReg,0x80);

    if (i!=0)
    {    
        if(!(ReadRawRC(ErrorReg)&0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {   status = MI_NOTAGERR;   }
            if (Command == PCD_TRANSCEIVE)
            {
               	n = ReadRawRC(FIFOLevelReg);
              	lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits)
                {   *pOutLenBit = (n-1)*8 + lastBits;   }
                else
                {   *pOutLenBit = n*8;   }
                if (n == 0)
                {   n = 1;    }
                if (n > MAXRLEN)
                {   n = MAXRLEN;   }
                for (i=0; i<n; i++)
                {   pOut [i] = ReadRawRC(FIFODataReg);    }
            }
        }
        else
        {   status = MI_ERR;   }
        
    }
   
    SetBitMask(ControlReg,0x80);           // stop timer now
    WriteRawRC(CommandReg,PCD_IDLE); 
    return status;
}

/////////////////////////////////////////////////////////////////////
//开启天线  
//每次启动或关闭天险发射之间应至少有1ms的间隔
/////////////////////////////////////////////////////////////////////
void PcdAntennaOn(void)
{
    u8   i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}


/////////////////////////////////////////////////////////////////////
//关闭天线
/////////////////////////////////////////////////////////////////////
void PcdAntennaOff(void)
{
	ClearBitMask(TxControlReg, 0x03);
}


//扫描卡片  串口打印id
u8 scan_card(unsigned char * ID)
{
			status = PcdRequest(PICC_REQALL,CT);//扫描卡
			status = PcdAnticoll(SN);/*防冲撞*/
			

	if (status==MI_OK)
			{		
				strcpy(ID,SN);				
				memset(SN,0,4);		
				return 1;	
			}
			
		
	return 0;
}