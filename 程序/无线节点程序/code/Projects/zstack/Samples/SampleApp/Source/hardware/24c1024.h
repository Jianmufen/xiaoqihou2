#ifndef __24C1024_H
#define __24C1024_H

#include "i2c.h"



#define WADDR 0xAC       //24c1024写地址（由地址线A1，A2硬件接线决定）
#define PAGELENGTH 0x100 //24c1024页长度为256字节


/*24c1024地址定义*/
#define EEP_BASE_ADDR 0x0000   //24c1024读写基地址
#define EEP_ADDR_128  (EEP_BASE_ADDR+128)   //128字节处
#define EEP_ADDR_256  (EEP_BASE_ADDR+256)   //256字节处
#define EEP_ADDR_384  (EEP_BASE_ADDR+384)   //384字节处
#define EEP_ADDR_512  (EEP_BASE_ADDR+512)   //512字节处
#define EEP_ADDR_640  (EEP_BASE_ADDR+640)   //640字节处
#define EEP_ADDR_768  (EEP_BASE_ADDR+768)   //768字节处
#define EEP_ADDR_896  (EEP_BASE_ADDR+896)   //896字节处
#define EEP_ADDR_1024 (EEP_BASE_ADDR+1024)  //1024字节处



/*使用到的地址定义*/
#define TEMP_CAL_ADDR  (EEP_BASE_ADDR)   //温度修正值存储地址
#define BAUD_ADDR      (TEMP_CAL_ADDR+sizeof(CAL_TYPE))  //串口波特率存储地址  /*不用*/
#define LG_LT_ADDR     (BAUD_ADDR+sizeof(uint8))         //辐射经纬度存储地址

//质控参数存储地址
#define QC_1ST_ADDR    (EEP_ADDR_128)    //第一个质控参数存储地址
#define QC_2ND_ADDR    (QC_1ST_ADDR+sizeof(QC_TYPE))    //第二个质控参数存储地址
#define QC_3RD_ADDR    (QC_2ND_ADDR+sizeof(QC_TYPE))    //第三个质控参数存储地址
#define QC_4TH_ADDR    (QC_3RD_ADDR+sizeof(QC_TYPE))    //第四个质控参数存储地址
#define QC_5TH_ADDR    (QC_4TH_ADDR+sizeof(QC_TYPE))    //第五个质控参数存储地址

//CR校正参数存储地址，气温，地温和湿度用到
#define CR_1ST_ADDR    (EEP_ADDR_256)
#define CR_2ND_ADDR    (CR_1ST_ADDR+sizeof(Quad_Cal_Coef_t))
#define CR_3RD_ADDR    (CR_2ND_ADDR+sizeof(Quad_Cal_Coef_t))
#define CR_4TH_ADDR    (CR_3RD_ADDR+sizeof(Quad_Cal_Coef_t))
#define CR_5TH_ADDR    (CR_4TH_ADDR+sizeof(Quad_Cal_Coef_t))

/* 2015.12.3添加设备状态值范围可设 */
//板温范围存储地址
#define BT_ADDR        (CR_5TH_ADDR+sizeof(Quad_Cal_Coef_t))
//电池电压范围存储地址
#define BV_ADDR        (BT_ADDR+sizeof(DeviceStateRangeTypeDef))






bool EEP_ReadBytes(unsigned char *pBuffer,unsigned char length,unsigned short ReadAddress);  //从24c1024一个地址开始读取length个字节
bool EEP_BuffWrite(unsigned char* pBuffer,unsigned char length,unsigned short WriteAddress);  //往24c1024一个地址开始写入length个字节
bool EEP_WriteByte(unsigned char SendByte,unsigned short WriteAddress);  //往24c1024一个地址中写入一个字节


#endif
