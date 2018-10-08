#include "sys.h"
#include "rc522.h"

unsigned char CT[2];//������
unsigned char SN[4]; //����
unsigned char status;
int spi_slave=0;  //SPI���豸ѡ�� ���ݴ˱���ѡ��Ƭѡ�ź�PBX  

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
	while((SPI2->SR&0X02)==0);		//�ȴ���������	  
	SPI2->DR=Byte;	 	//����һ��byte   
	while((SPI2->SR&0X01)==0);//�ȴ�������һ��byte  
	return SPI2->DR;          	     //�����յ�������			
}
void SPI2_Init(void)	
{ 
	//����SPI2�ܽ�

	RCC->APB2ENR|=1<<0;
	RCC->APB2ENR|=1<<3;
	RCC->APB2ENR|=1<<5;
	RCC->APB2ENR|=1<<6;
	
	GPIOB->CRH&=0X000F00FF;		
	GPIOB->CRH|=0XB8B03300;
	
	GPIOD->CRH&=0XFFFF0FFF;
	GPIOD->CRH|=0X00003000;

	GPIOB->CRL&=0XFFFFF000;		 //PB.1 ��λ
	GPIOB->CRL|=0X00000333;		//PB.0 Ƭѡ �������

	GPIOE->CRL&=0XFFFFFFF0;		//��ʼ��PE13 PE15 PE14 PE11Ƭѡ����
	GPIOE->CRL|=0X00000003;			//��ʼ��PE0 ָʾ
	
	GPIOE->CRH&=0XFFFFFFF0;		//��ʼ��PE13 PE15 PE14 PE11Ƭѡ����
	GPIOE->CRH|=0X00000003;

	

 PBout(10) =1;
 
	RCC->APB1ENR|=1<<14;

	SPI2->CR1|=0<<10;//ȫ˫��ģʽ	
	SPI2->CR1&=0<<9; //���nss����
	SPI2->CR1|=1<<8;  
	
	SPI2->CR1|=1<<2; //SPI����
	SPI2->CR1|=0<<11;//8bit���ݸ�ʽ	
	SPI2->CR1&=~0x02; //����ģʽ��SCKΪ0 CPOL=0
	SPI2->CR1&=~0x01; //���ݲ����ӵ�һ��ʱ����ؿ�ʼ,CPHA=0  
	SPI2->CR1|=7<<3; //Fsck=Fcpu/4
	SPI2->CR1|=0<<7; //MSBfirst   
	SPI2->CR1|=1<<6; //SPI�豸ʹ�� 
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
//��    �ܣ�Ѱ��
//����˵��: req_code[IN]:Ѱ����ʽ
//                0x52 = Ѱ��Ӧ�������з���14443A��׼�Ŀ�
//                0x26 = Ѱδ��������״̬�Ŀ�
//          pTagType[OUT]����Ƭ���ʹ���
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ�����ײ
//����˵��: pSnr[OUT]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ�ѡ����Ƭ
//����˵��: pSnr[IN]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ����Ƭ��������״̬
//��    ��: �ɹ�����MI_OK
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
//��MF522����CRC16����
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
//��    �ܣ���λRC522
//��    ��: �ɹ�����MI_OK
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
    
    WriteRawRC(ModeReg,0x3D);            //��Mifare��ͨѶ��CRC��ʼֵ0x6363
    WriteRawRC(TReloadRegL,30);           
    WriteRawRC(TReloadRegH,0);
    WriteRawRC(TModeReg,0x8D);
    WriteRawRC(TPrescalerReg,0x3E);
	
		WriteRawRC(TxAutoReg,0x40);//����Ҫ
   
    return MI_OK;
}

//////////////////////////////////////////////////////////////////////
//����RC632�Ĺ�����ʽ 
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
//��    �ܣ���RC522�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//��    �أ�������ֵ
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
				
				//����Ƭѡ�󣬱���100ns��
				//�μ�RC522�����ֲ�P90��Tsclkl����
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
//��    �ܣ�дRC522�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//          value[IN]:д���ֵ
/////////////////////////////////////////////////////////////////////
void WriteRawRC(u8   Address, u8   value)
{  
    u8   ucAddr;

	switch(spi_slave)
	{
		case 0: 
		{
			SPI_CS0=0;
			
			//����Ƭѡ�󣬱���100ns��
			//�μ�RC522�����ֲ�P90��Tsclkl����
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
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
/////////////////////////////////////////////////////////////////////
void SetBitMask(u8   reg,u8   mask)  
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
/////////////////////////////////////////////////////////////////////
void ClearBitMask(u8   reg,u8   mask)  
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
} 

/////////////////////////////////////////////////////////////////////
//��    �ܣ�ͨ��RC522��ISO14443��ͨѶ
//����˵����Command[IN]:RC522������
//          pIn [IN]:ͨ��RC522���͵���Ƭ������
//          InLenByte[IN]:�������ݵ��ֽڳ���
//          pOut [OUT]:���յ��Ŀ�Ƭ��������
//          *pOutLenBit[OUT]:�������ݵ�λ����
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
    ClearBitMask(ComIrqReg,0x80);	//�������ж�λ
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);	 	//��FIFO����
    
    for (i=0; i<InLenByte; i++)
    {   WriteRawRC(FIFODataReg, pIn [i]);    }
    WriteRawRC(CommandReg, Command);	  
//   	 n = ReadRawRC(CommandReg);
    
    if (Command == PCD_TRANSCEIVE)
    {    SetBitMask(BitFramingReg,0x80);  }	 //��ʼ����
    										 
    //i = 600;//����ʱ��Ƶ�ʵ���������M1�����ȴ�ʱ��25ms
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
//��������  
//ÿ��������ر����շ���֮��Ӧ������1ms�ļ��
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
//�ر�����
/////////////////////////////////////////////////////////////////////
void PcdAntennaOff(void)
{
	ClearBitMask(TxControlReg, 0x03);
}


//ɨ�迨Ƭ  ���ڴ�ӡid
u8 scan_card(unsigned char * ID)
{
			status = PcdRequest(PICC_REQALL,CT);//ɨ�迨
			status = PcdAnticoll(SN);/*����ײ*/
			

	if (status==MI_OK)
			{		
				strcpy(ID,SN);				
				memset(SN,0,4);		
				return 1;	
			}
			
		
	return 0;
}