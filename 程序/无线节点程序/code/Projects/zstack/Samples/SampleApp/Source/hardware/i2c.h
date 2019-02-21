#ifndef __I2C_H
#define __I2C_H

#include "ioCC2530.h"
#include "OSAL.h"
#include "hal_board.h"
#include "OnBoard.h"
#include "hal_defs.h"


#define CLK  P0_4  //I2Cʱ����
#define SSDA P0_5  //I2C������

#define SDA_O()  //st( P0DIR|=1<<5;   )   //��I2C�����߷�������Ϊ���  /*������ʱ������Ӧԭ����32768����ƥ����ݺ�����18p����������������io�ڷ����£�ȥ��io�������ú���Ȼ����*/
#define SDA_I()  //st( P0DIR&=~(1<<5);)   //��I2C�����߷�������Ϊ����


//ģ��I2C����������������I2C����
void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_delay(void);
bool I2C_WaitAck(void);
void I2C_Ack(void);
void I2C_NoAck(void);
unsigned char I2C_ReceiveByte(void);
unsigned char I2C_SendByte(unsigned char cmd);

//��ָ����ַ��ȡcount���ֽڣ�������24c1024��pcf8563��I2C����
bool ReadData_s(unsigned char address,unsigned char count,unsigned char *buf);


#endif

