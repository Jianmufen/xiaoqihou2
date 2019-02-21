#include "i2c.h"


/******************************************************************************
 *
 *                ģ��I2C���ߺ���
 *
 */

void I2C_delay(void)  //PCF8563��I2C����Ƶ��Ϊ400kHz�����ݲ�ͬ�������趨�ʵ��ȴ���
{
  MicroWait(5);   //�������Ż��ٶ�
//  unsigned char i=5;
//  while(i)
//  {
//    i--;
//  }
}

void I2C_Start(void)
{	
	//EA=0;
        SDA_O();  //����Ϊ���
	SSDA=1;
	CLK=1;
	I2C_delay();
	SSDA=0;
	I2C_delay();
	CLK=0;
	I2C_delay();
}

void I2C_Stop(void)
{
        SDA_O();  //����Ϊ���
	SSDA=0;
	CLK=1;
	I2C_delay();
	SSDA=1;
	I2C_delay();
	CLK=0;
	I2C_delay();
	//EA=1;
}

unsigned char I2C_SendByte(unsigned char cmd)  //����һ���ֽ�
{
	unsigned char i=8;
        SDA_O();  //����Ϊ���
	while(i--)
	{
		CLK=0;
		if(cmd&0x80)
		{
			SSDA=1;
		}
		else
		{
			SSDA=0;
		}
		cmd<<=1;
		I2C_delay();
		CLK=1;
		I2C_delay();
	}
        SDA_I();  //����Ϊ����
	CLK=0;
	return SSDA;
}

unsigned char I2C_ReceiveByte(void)  //��ȡһ���ֽ�
{
	unsigned char i=8;
	unsigned char ReceiveByte=0;
        SDA_O();  //����Ϊ���
	SSDA=1;
        SDA_I();  //����Ϊ����
	while(i--)
	{
		ReceiveByte<<=1;
		CLK=0;
		I2C_delay();
		CLK=1;
		I2C_delay();
		if(SSDA)
		{
			ReceiveByte|=0x01;
		}
	}
	CLK=0;
	return ReceiveByte;
}

bool I2C_WaitAck(void)  //�ȴ�I2C����Ӧ��
{
        SDA_O();  //����Ϊ���
	CLK=0;
	I2C_delay();
	SSDA=1;
	I2C_delay();
	CLK=1;
	I2C_delay();
        SDA_I();  //����Ϊ����
	if(SSDA)
	{
		CLK=0;
		return FALSE;
	}
	CLK=0;
	return TRUE;
}

void I2C_Ack(void)  //����I2CӦ��
{
        SDA_O();  //����Ϊ���
	CLK=0;
	I2C_delay();
	SSDA=0;
	I2C_delay();
	CLK=1;
	I2C_delay();
	CLK=0;
	I2C_delay();
}

void I2C_NoAck(void) //����I2CӦ��ʧ��
{
        SDA_O();  //����Ϊ���
	CLK=0;
	I2C_delay();
	SSDA=1;
	I2C_delay();
	CLK=1;
	I2C_delay();
	CLK=0;
	I2C_delay();
}

/************************************************************************
 *    ģ��I2C
 */

