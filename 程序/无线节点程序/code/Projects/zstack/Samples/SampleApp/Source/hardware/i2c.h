#ifndef __I2C_H
#define __I2C_H

#include "ioCC2530.h"
#include "OSAL.h"
#include "hal_board.h"
#include "OnBoard.h"
#include "hal_defs.h"


#define CLK  P0_4  //I2C时钟线
#define SSDA P0_5  //I2C数据线

#define SDA_O()  //st( P0DIR|=1<<5;   )   //将I2C数据线方向配置为输出  /*经测试时钟无响应原因是32768晶振匹配电容焊错，换18p后正常，不是由于io口方向导致，去掉io方向配置后仍然正常*/
#define SDA_I()  //st( P0DIR&=~(1<<5);)   //将I2C数据线方向配置为输入


//模拟I2C函数，可用于其他I2C器件
void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_delay(void);
bool I2C_WaitAck(void);
void I2C_Ack(void);
void I2C_NoAck(void);
unsigned char I2C_ReceiveByte(void);
unsigned char I2C_SendByte(unsigned char cmd);

//从指定地址读取count和字节，可用于24c1024和pcf8563等I2C器件
bool ReadData_s(unsigned char address,unsigned char count,unsigned char *buf);


#endif

