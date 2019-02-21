#ifndef __RTC_H
#define __RTC_H

#include "ioCC2530.h"
#include "OSAL.h"
#include "hal_board.h"
#include "OnBoard.h"
#include "SampleApp.h"
#include "hal_defs.h"
#include "i2c.h"


//#define CLK  P0_4  //I2Cʱ����
//#define SSDA P0_5  //I2C������
//
//#define SDA_O()  //st( P0DIR|=1<<5;   )   //��I2C�����߷�������Ϊ���  /*������ʱ������Ӧԭ����32768����ƥ����ݺ�����18p����������������io�ڷ����£�ȥ��io�������ú���Ȼ����*/
//#define SDA_I()  //st( P0DIR&=~(1<<5);)   //��I2C�����߷�������Ϊ����

//PCF8563�Ĵ�����ַ���趨�����
#define RTC_WR_ADD          0xA2    //д����
#define RTC_RD_ADD          0xA3    //������
#define RTC_CON_REG_ADD     0x00    //���ƼĴ���1��ַ
#define RTC_ON_DAT          0x00    //����RTC����
#define RTC_IRQ_ON_DAT      0x13    //�ж�����
#define RTC_ALARM_REG_ADD   0x09    //���ӱ����Ĵ�����ַ
#define RTC_ALARM_OFF_DAT   0x80    //�رձ���
#define RTC_OUT_OFF_DAT     0x00    //�ر�CLKOUT���
#define RTC_TIM_ON_DAT      0x82    //����������ʱ�����趨����Ƶ��
#define RTC_TIM_COUNT_DAT   0x01    //��������ʱ��װ��ֵ

//��ʱ������Ƶ��ѡ��
#define RTC_TIM_ON_4096HZ   0x80    //4096Hz
#define RTC_TIM_ON_64HZ     0x81    //64Hz
#define RTC_TIM_ON_1HZ      0x82    //1Hz
#define RTC_TIM_ON_1_60HZ   0x83    //1/60Hz





#define BCD_TO_DEC(x)  ((((x)>>4)&0x0F)*10+((x)&0x0F))   //BCDת10����
#define DEC_TO_BCD(x)  ((((x)/10)<<4)+(((x)%10)&0x0F))   //10����תBCD

//����
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
  
//ʱ��ṹ��
typedef struct
{
  uint8 year;  //��
  uint8 month; //��
  uint8 day;   //��
  uint8 week;  //����
  uint8 hour;  //ʱ
  uint8 min;   //��
  uint8 sec;   //��
}RTC_TIME_TYPE;
extern RTC_TIME_TYPE sys_time;

extern RTC_TIME_TYPE res_time;  //���ڱ���ĳһ�¼�ǰ��ʱ��

//RTC����
void RTC_Init(void);  //RTC��ʼ��
uint8 RTC_SetTime(RTC_TIME_TYPE *time);  //����ʱ��
uint8 RTC_GetTime(RTC_TIME_TYPE *time);  //��ȡʱ��


////ģ��I2C����������������I2C����
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
////��ָ����ַ��ȡcount���ֽڣ�������24c1024��pcf8563��I2C����
//bool ReadData_s(unsigned char address,unsigned char count,unsigned char *buf);




//����ʱ����㺯��
unsigned long
l_mktime(const unsigned int year0, const unsigned int mon0,
const unsigned int day, const unsigned int hour,
const unsigned int min, const unsigned int sec);  //�����1970��1��1��0ʱ������ʱ�侭��������

int isLeapYear(int year);  //�ж�һ������Ƿ�Ϊ����

void AddaMinute(int *year, int *month, int *day, int *hour, int *minute, int *second);  //��һ����������1����
void Add5Minute(int *year, int *month, int *day, int *hour, int *minute, int *second); //��һ������ʱ������5����
void AddaHour(int *year, int *month, int *day, int *hour, int *minute, int *second);  //��һ������ʱ������1Сʱ




#endif

