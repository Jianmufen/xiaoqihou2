#include "i2c.h"


/******************************************************************************
 *
 *                模拟I2C总线函数
 *
 */

void I2C_delay(void)  //PCF8563的I2C总线频率为400kHz，根据不同处理器设定适当等待数
{
  MicroWait(5);   //在这里优化速度
//  unsigned char i=5;
//  while(i)
//  {
//    i--;
//  }
}

void I2C_Start(void)
{	
	//EA=0;
        SDA_O();  //配置为输出
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
        SDA_O();  //配置为输出
	SSDA=0;
	CLK=1;
	I2C_delay();
	SSDA=1;
	I2C_delay();
	CLK=0;
	I2C_delay();
	//EA=1;
}

unsigned char I2C_SendByte(unsigned char cmd)  //发送一个字节
{
	unsigned char i=8;
        SDA_O();  //配置为输出
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
        SDA_I();  //配置为输入
	CLK=0;
	return SSDA;
}

unsigned char I2C_ReceiveByte(void)  //读取一个字节
{
	unsigned char i=8;
	unsigned char ReceiveByte=0;
        SDA_O();  //配置为输出
	SSDA=1;
        SDA_I();  //配置为输入
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

bool I2C_WaitAck(void)  //等待I2C总线应答
{
        SDA_O();  //配置为输出
	CLK=0;
	I2C_delay();
	SSDA=1;
	I2C_delay();
	CLK=1;
	I2C_delay();
        SDA_I();  //配置为输入
	if(SSDA)
	{
		CLK=0;
		return FALSE;
	}
	CLK=0;
	return TRUE;
}

void I2C_Ack(void)  //发送I2C应答
{
        SDA_O();  //配置为输出
	CLK=0;
	I2C_delay();
	SSDA=0;
	I2C_delay();
	CLK=1;
	I2C_delay();
	CLK=0;
	I2C_delay();
}

void I2C_NoAck(void) //发送I2C应答失败
{
        SDA_O();  //配置为输出
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
 *    模拟I2C
 */

