#include "24c1024.h"
#include "stdio.h"


//��24c1024һ����ַ��д��һ���ֽ�
bool EEP_WriteByte(unsigned char SendByte,unsigned short WriteAddress)
{
	unsigned char DeviceAddress=WADDR;
	I2C_Start();
	I2C_SendByte(DeviceAddress);    //д��ַ
	if(!I2C_WaitAck())   //�ȴ������ظ�
	{
		I2C_Stop();
		return FALSE;
	}
	I2C_SendByte((unsigned char )((WriteAddress&0xFF00)>8));   //����Ҫ��д��16λ��ַ
	I2C_WaitAck();
	I2C_SendByte((unsigned char )(WriteAddress&0x00FF));
	I2C_WaitAck();
	I2C_SendByte(SendByte);     //����Ҫд����ֽ�
	I2C_WaitAck();
	I2C_Stop();
	MicroWait(5000);  //�ȴ�5ms
	return TRUE;
}

//��24c1024һ����ַ��ʼд��length���ֽ�
bool EEP_BuffWrite(unsigned char* pBuffer,unsigned char length,unsigned short WriteAddress)
{
	unsigned char DeviceAddress=WADDR;
	I2C_Start();
	I2C_SendByte(DeviceAddress);    //д��ַ
	if(!I2C_WaitAck())   //�ȴ������ظ�
	{
		I2C_Stop();
		return FALSE;
	}
	I2C_SendByte((unsigned char )((WriteAddress&0xFF00)>8));   //����Ҫ��д��16λ��ַ
	I2C_WaitAck();
	I2C_SendByte((unsigned char )(WriteAddress&0x00FF));
	I2C_WaitAck();
	while(length--)   //����Ҫд����ֽ�
	{
		I2C_SendByte(*pBuffer);
		I2C_WaitAck();
		pBuffer++;
	}
	I2C_Stop();
	MicroWait(5000);  //�ȴ�5ms
	return TRUE;
}

#if 0
//д��һҳ
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
	MicroWait(5000);  //�ȴ�5ms
	return TRUE;
}
#endif

//��24c1024һ����ַ��ʼ��ȡlength���ֽ�
bool EEP_ReadBytes(unsigned char *pBuffer,unsigned char length,unsigned short ReadAddress)
{
	unsigned char DeviceAddress=WADDR;
	I2C_Start();
	I2C_SendByte(DeviceAddress);   //��ȡʱҪ����д����
	if(!I2C_WaitAck())
	{
		I2C_Stop();
		return FALSE;
	}
	I2C_SendByte((unsigned char)((ReadAddress&0xFF00)>>8));  //�ٷ��Ͷ�ȡ��ַ
	I2C_WaitAck();
	I2C_SendByte((unsigned char)(ReadAddress&0x00FF));
	I2C_WaitAck();
	I2C_Start();
	I2C_SendByte(DeviceAddress|0x01);  //���ŷ��Ͷ�������ж�ȡ
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



