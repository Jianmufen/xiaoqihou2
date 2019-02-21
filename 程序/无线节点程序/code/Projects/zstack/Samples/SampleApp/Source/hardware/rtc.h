#ifndef __RTC_H
#define __RTC_H

#include "ioCC2530.h"
#include "OSAL.h"
#include "hal_board.h"
#include "OnBoard.h"
#include "SampleApp.h"
#include "hal_defs.h"
#include "i2c.h"


//#define CLK  P0_4  //I2C时钟线
//#define SSDA P0_5  //I2C数据线
//
//#define SDA_O()  //st( P0DIR|=1<<5;   )   //将I2C数据线方向配置为输出  /*经测试时钟无响应原因是32768晶振匹配电容焊错，换18p后正常，不是由于io口方向导致，去掉io方向配置后仍然正常*/
//#define SDA_I()  //st( P0DIR&=~(1<<5);)   //将I2C数据线方向配置为输入

//PCF8563寄存器地址及设定命令定义
#define RTC_WR_ADD          0xA2    //写命令
#define RTC_RD_ADD          0xA3    //读命令
#define RTC_CON_REG_ADD     0x00    //控制寄存器1地址
#define RTC_ON_DAT          0x00    //启动RTC命令
#define RTC_IRQ_ON_DAT      0x13    //中断配置
#define RTC_ALARM_REG_ADD   0x09    //分钟报警寄存器地址
#define RTC_ALARM_OFF_DAT   0x80    //关闭报警
#define RTC_OUT_OFF_DAT     0x00    //关闭CLKOUT输出
#define RTC_TIM_ON_DAT      0x82    //开倒计数定时器并设定计数频率
#define RTC_TIM_COUNT_DAT   0x01    //倒计数定时器装载值

//定时器计数频率选择
#define RTC_TIM_ON_4096HZ   0x80    //4096Hz
#define RTC_TIM_ON_64HZ     0x81    //64Hz
#define RTC_TIM_ON_1HZ      0x82    //1Hz
#define RTC_TIM_ON_1_60HZ   0x83    //1/60Hz





#define BCD_TO_DEC(x)  ((((x)>>4)&0x0F)*10+((x)&0x0F))   //BCD转10进制
#define DEC_TO_BCD(x)  ((((x)/10)<<4)+(((x)%10)&0x0F))   //10进制转BCD

//星期
typedef enum
{
  SUN=0,
  MON,
  TUE,
  WED,
  THU,
  FRI,
  SAT
}WEEK_TYPE;
  
//时间结构体
typedef struct
{
  uint8 year;  //年
  uint8 month; //月
  uint8 day;   //日
  uint8 week;  //星期
  uint8 hour;  //时
  uint8 min;   //分
  uint8 sec;   //秒
}RTC_TIME_TYPE;
extern RTC_TIME_TYPE sys_time;

extern RTC_TIME_TYPE res_time;  //用于保存某一事件前的时间

//RTC函数
void RTC_Init(void);  //RTC初始化
uint8 RTC_SetTime(RTC_TIME_TYPE *time);  //设置时间
uint8 RTC_GetTime(RTC_TIME_TYPE *time);  //获取时间


////模拟I2C函数，可用于其他I2C器件
//void I2C_Init(void);
//void I2C_Start(void);
//void I2C_Stop(void);
//void I2C_delay(void);
//bool I2C_WaitAck(void);
//void I2C_Ack(void);
//void I2C_NoAck(void);
//unsigned char I2C_ReceiveByte(void);
//unsigned char I2C_SendByte(unsigned char cmd);
//
////从指定地址读取count和字节，可用于24c1024和pcf8563等I2C器件
//bool ReadData_s(unsigned char address,unsigned char count,unsigned char *buf);




//其它时间计算函数
unsigned long
l_mktime(const unsigned int year0, const unsigned int mon0,
const unsigned int day, const unsigned int hour,
const unsigned int min, const unsigned int sec);  //计算从1970年1月1日0时到给定时间经过的秒数

int isLeapYear(int year);  //判断一个年份是否为闰年

void AddaMinute(int *year, int *month, int *day, int *hour, int *minute, int *second);  //将一个日期增加1分钟
void Add5Minute(int *year, int *month, int *day, int *hour, int *minute, int *second); //将一个日期时间增加5分钟
void AddaHour(int *year, int *month, int *day, int *hour, int *minute, int *second);  //将一个日期时间增加1小时




#endif

