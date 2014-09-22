#include"u.h"

//****************************************IO�˿ڶ���***************************************
sbit 	MISO	=P2^5;//�������
sbit 	MOSI	=P2^4;//��������
sbit	SCK	    =P2^6;  //SPI CLOCK 
sbit	CE	    =P2^7; //  ʹ��ʱ��
sbit	CSN		=P2^3; // SPI chip ѡ��
sbit	IRQ		=P2^2; //  
sbit led = P2^1;
uchar code TX_ADDRESS[TX_ADR_WIDTH]= {0x34,0x43,0x10,0x10,0x01};	//���ص�ַ
uchar code RX_ADDRESS[RX_ADR_WIDTH]= {0x34,0x43,0x10,0x10,0x01};	//���յ�ַ
//*****************************************����ʱ*****************************************
void Delay_2(unsigned int s)
{
	unsigned int i;
	for(i=0; i<s; i++);
	for(i=0; i<s; i++);
}
//******************************************************************************************
uint 	bdata sta;   //״̬��־
sbit	RX_DR	=sta^6;
sbit	TX_DS	=sta^5;
sbit	MAX_RT	=sta^4;
void Delay_ms(uint ms)
{
	unsigned char a,b,c;
	for(ms;ms>0;ms--)
	{
		 for(c=1;c>0;c--)
			for(b=142;b>0;b--)
				for(a=2;a>0;a--);
	}

}

void Delay_100us(uchar us)
{
	uchar a,b;
	for(us;us>0;us--)
	{
		for(b=1;b>0;b--)
        	for(a=47;a>0;a--);
	}
}
/******************************************************************************************
/*��ʱ����
/******************************************************************************************/
void inerDelay_us(unsigned char n)
{
	for(;n>0;n--)
		_nop_();
}
//****************************************************************************************
/*NRF24L01��ʼ��
//***************************************************************************************/
void init_NRF24L01(void)
{
    inerDelay_us(100);
 	CE=0;    // chip enable
 	CSN=1;   // Spi disable 
 	SCK=0;   // Spi clock line init high
	SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);    // д���ص�ַ	
	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); // д���ն˵�ַ
	SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);      //  Ƶ��0�Զ�	ACKӦ������	
	SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);  //  ������յ�ַֻ��Ƶ��0�������Ҫ��Ƶ�����Բο�Page21  
	SPI_RW_Reg(WRITE_REG + RF_CH, 0);        //   �����ŵ�����Ϊ2.4GHZ���շ�����һ��
	SPI_RW_Reg(WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH); //���ý������ݳ��ȣ���������Ϊ32�ֽ�
	SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x07);   		//���÷�������Ϊ1MHZ�����书��Ϊ���ֵ0dB
	SPI_RW_Reg(WRITE_REG+SETUP_RETR,0x0a);	
   //	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e);   		 // IRQ�շ�����ж���Ӧ��16λCRC��������
   
}
/****************************************************************************************************
/*������uint SPI_RW(uint uchar1)
/*���ܣ�NRF24L01��SPIдʱ��
/****************************************************************************************************/
uint SPI_RW(uint uchar1)
{
	uint bit_ctr;
   	for(bit_ctr=0;bit_ctr<8;bit_ctr++) // output 8-bit
   	{
		MOSI = (uchar1 & 0x80);         // output 'uchar', MSB to MOSI
		uchar1 = (uchar1 << 1);           // shift next bit into MSB..
		SCK = 1;                      // Set SCK high..
		uchar1 |= MISO;       		  // capture current MISO bit
		SCK = 0;            		  // ..then set SCK low again
   	}
    return(uchar1);           		  // return read uchar
}
/****************************************************************************************************
/*������uchar SPI_Read(uchar reg)
/*���ܣ�NRF24L01��SPIʱ��
/****************************************************************************************************/
uchar SPI_Read(uchar reg)
{
	uchar reg_val;
	
	CSN = 0;                // CSN low, initialize SPI communication...
	SPI_RW(reg);            // Select register to read from..
	reg_val = SPI_RW(0);    // ..then read registervalue
	CSN = 1;                // CSN high, terminate SPI communication
	
	return(reg_val);        // return register value
}
/****************************************************************************************************/
/*���ܣ�NRF24L01��д�Ĵ�������
/****************************************************************************************************/
uint SPI_RW_Reg(uchar reg, uchar value)
{
	uint status;
	
	CSN = 0;                   // CSN low, init SPI transaction
	status = SPI_RW(reg);      // select register
	SPI_RW(value);             // ..and write value to it..
	CSN = 1;                   // CSN high again
	
	return(status);            // return nRF24L01 status uchar
}
/****************************************************************************************************/
/*������uint SPI_Read_Buf(uchar reg, uchar *pBuf, uchar uchars)
/*����: ���ڶ����ݣ�reg��Ϊ�Ĵ�����ַ��pBuf��Ϊ���������ݵ�ַ��uchars���������ݵĸ���
/****************************************************************************************************/
uint SPI_Read_Buf(uchar reg, uchar *pBuf, uchar uchars)
{
	uint status,uchar_ctr;
	
	CSN = 0;                    		// Set CSN low, init SPI tranaction
	status = SPI_RW(reg);       		// Select register to write to and read status uchar
	
	for(uchar_ctr=0;uchar_ctr<uchars;uchar_ctr++)
		pBuf[uchar_ctr] = SPI_RW(0);    // 
	
	CSN = 1;                           
	
	return(status);                    // return nRF24L01 status uchar
}
/*********************************************************************************************************
/*������uint SPI_Write_Buf(uchar reg, uchar *pBuf, uchar uchars)
/*����: ����д���ݣ�Ϊ�Ĵ�����ַ��pBuf��Ϊ��д�����ݵ�ַ��uchars��д�����ݵĸ���
/*********************************************************************************************************/
uint SPI_Write_Buf(uchar reg, uchar  *pBuf, uchar uchars)
{
	uint status,uchar_ctr;
	
	CSN = 0;            //SPIʹ��       
	status = SPI_RW(reg);   
	for(uchar_ctr=0; uchar_ctr<uchars; uchar_ctr++) //
		SPI_RW(*pBuf++);
	CSN = 1;           //�ر�SPI
	return(status);    // 
}
/****************************************************************************************************/
/*������void SetRX_Mode(void)
/*���ܣ����ݽ������� 
/****************************************************************************************************/
void SetRX_Mode(void)
{
	CE=0;
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);   		// IRQ�շ�����ж���Ӧ��16λCRC	��������
	CE = 1; 
	inerDelay_us(130);
}
/******************************************************************************************************/
/*������unsigned char nRF24L01_RxPacket(unsigned char* rx_buf)
/*���ܣ����ݶ�ȡ�����rx_buf���ջ�������
/******************************************************************************************************/
unsigned char nRF24L01_RxPacket(unsigned char* rx_buf)
{
    unsigned char revale=0;
	sta=SPI_Read(STATUS);	// ��ȡ״̬�Ĵ������ж����ݽ���״��
	if(RX_DR)				// �ж��Ƿ���յ�����
	{  
	    CE = 0; 			//SPIʹ��
		SPI_Read_Buf(RD_RX_PLOAD,rx_buf,TX_PLOAD_WIDTH);// read receive payload from RX_FIFO buffer
		revale =1;			//��ȡ������ɱ�־
	}
	SPI_RW_Reg(WRITE_REG+STATUS,sta);   //���յ����ݺ�RX_DR,TX_DS,MAX_PT���ø�Ϊ1��ͨ��д1������жϱ�־
	return revale;
} 
/***********************************************************************************************************
/*������void nRF24L01_TxPacket(unsigned char * tx_buf)
/*���ܣ����� tx_buf������
/**********************************************************************************************************/
void NRF24L01_TxPacket(unsigned char * tx_buf)
{	
	CE=0;			//StandBy Iģʽ	
	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // װ�ؽ��ն˵�ַ		
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e);   		 // IRQ�շ�����ж���Ӧ��16λCRC��������
	SPI_Write_Buf(WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH); 			 // װ������
	CE=1;		 //�ø�CE���������ݷ���

Delay_ms(10);
}
void check_ACK()
{
	uchar test;
	test=SPI_Read(READ_REG+STATUS);	// read register STATUS's
	test=test&0x20;					// check if have Data sent TX FIFO interrupt (TX_DS=1)
	if(test==0x20)					// TX_DS =1
	{

 	 /******����������Ѳο���һƪ�ĵ��ӵ�*/
		CSN = 0;
		SPI_RW(FLUSH_TX);
   		CSN = 1;
 	 /**����Ϊֹ*/
	 	SPI_RW_Reg(WRITE_REG+STATUS,0xff);//���жϱ�־
	}
}  

void main()
{	
  	while(1)
	{
	    init_NRF24L01();
	  	CE=1;
	    NRF24L01_TxPacket(Txbuf);
	    Delay_ms(10);
		check_ACK();//����Ӧ���źż�⣬LED��˸��־���ͳɹ�  
	}				
}