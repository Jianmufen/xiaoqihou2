#include "24c1024.h"
#include "stdio.h"


//往24c1024一个地址中写入一个字节
bool EEP_WriteByte(unsigned char SendByte,unsigned short WriteAddress)
{
	unsigned char DeviceAddress=WADDR;
	I2C_Start();
	I2C_SendByte(DeviceAddress);    //写地址
	if(!I2C_WaitAck())   //等待器件回复
	{
		I2C_Stop();
		return FALSE;
	}
	I2C_SendByte((unsigned char )((WriteAddress&0xFF00)>8));   //发送要读写的16位地址
	I2C_WaitAck();
	I2C_SendByte((unsigned char )(WriteAddress&0x00FF));
	I2C_WaitAck();
	I2C_SendByte(SendByte);     //发送要写入的字节
	I2C_WaitAck();
	I2C_Stop();
	MicroWait(5000);  //等待5ms
	return TRUE;
}

//往24c1024一个地址开始写入length个字节
bool EEP_BuffWrite(unsigned char* pBuffer,unsigned char length,unsigned short WriteAddress)
{
	unsigned char DeviceAddress=WADDR;
	I2C_Start();
	I2C_SendByte(DeviceAddress);    //写地址
	if(!I2C_WaitAck())   //等待器件回复
	{
		I2C_Stop();
		return FALSE;
	}
	I2C_SendByte((unsigned char )((WriteAddress&0xFF00)>8));   //发送要读写的16位地址
	I2C_WaitAck();
	I2C_SendByte((unsigned char )(WriteAddress&0x00FF));
	I2C_WaitAck();
	while(length--)   //发送要写入的字节
	{
		I2C_SendByte(*pBuffer);
		I2C_WaitAck();
		pBuffer++;
	}
	I2C_Stop();
	MicroWait(5000);  //等待5ms
	return TRUE;
}

#if 0
//写入一页
bool EEP_PageWrite(unsigned char* pBuffer,unsigned short WriteAddress)
{
	unsigned char DeviceAddress=WADDR;
	unsigned char length=PAGELENGTH;
	I2C_Start();
	I2C_SendByte(DeviceAddress);
	if(!I2C_WaitAck())
	{
		I2C_Stop();
		return FALSE;
	}
	I2C_SendByte((unsigned char )((WriteAddress&0xFF00)>>8));
	I2C_WaitAck();
	I2C_SendByte((unsigned char )(WriteAddress&0x00FF));
	I2C_WaitAck();
	while(length--)
	{
		I2C_SendByte(*pBuffer);
		I2C_WaitAck();
		pBuffer++;
	}
	I2C_Stop();
	MicroWait(5000);  //等待5ms
	return TRUE;
}
#endif

//从24c1024一个地址开始读取length个字节
bool EEP_ReadBytes(unsigned char *pBuffer,unsigned char length,unsigned short ReadAddress)
{
	unsigned char DeviceAddress=WADDR;
	I2C_Start();
	I2C_SendByte(DeviceAddress);   //读取时要发送写命令
	if(!I2C_WaitAck())
	{
		I2C_Stop();
		return FALSE;
	}
	I2C_SendByte((unsigned char)((ReadAddress&0xFF00)>>8));  //再发送读取地址
	I2C_WaitAck();
	I2C_SendByte((unsigned char)(ReadAddress&0x00FF));
	I2C_WaitAck();
	I2C_Start();
	I2C_SendByte(DeviceAddress|0x01);  //接着发送读命令进行读取
	I2C_WaitAck();
	while(length)
	{
		*pBuffer=I2C_ReceiveByte();
		if(length==1)
		{
			I2C_NoAck();
		}
		else I2C_Ack();
		pBuffer++;
		length--;
	}
	I2C_Stop();
	return TRUE;
}



