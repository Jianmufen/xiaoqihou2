#include "rtc.h"

#if (defined HAL_RTC) && (HAL_RTC == TRUE)

RTC_TIME_TYPE sys_time;  //���浱ǰϵͳʱ��
RTC_TIME_TYPE res_time;  //���ڱ���ĳһ�¼�ǰ��ʱ��



//�ڲ���������

/*bool RTC_WriteTime(unsigned char *time) ;  //δʹ�ã�����ʱ��ṹ���дʱ��*/




/******************************************************************************
 *
 *                ��ʼ��I2C����I/O�ں��ж�
 *
 */
void I2C_Init(void)  //ע��ģ��IICʱSSDA���������Ҳ������ڣ�Ҫע��ı��䷽�򣬷����޷���ȡ����
{ 
  P0SEL&=~(1<<4); //P0_4��P0_5��Ϊͨ������ڣ�����0��SS��C���Ų���ʱ���������ſ���ͨ�ÿڣ�
  P0SEL&=~(1<<5);
  P0DIR|=3<<4;  //ͨ�����
  
  //����P0_1ΪRTC�ж���������
  P0SEL&=~(1<<1); //P0_1��Ϊͨ��I/O��
  P0DIR&=~(1<<1); //����
  P0INP&=~(1<<1); //P0_1����������
  P2INP&=~(1<<5); //���� 
  //P2INP|=1<<5;  //����
  
  //�ⲿ�ж�����
  P0IEN|=1<<1;  //P0_1�ж�ʹ��
  PICTL|=0x01;  //P0���½��ش���
  //PICTL&=~0x01; //�����ش���
  IEN1|=1<<5;   //P0���ж�ʹ��
  P0IFG=0x00;   //�жϱ�־��ʼΪ0
  EA=1;   //�����ж�
  //MicroWait(100);
}

/******************************************************************************
 *
 *                ��ʼ��PCF8563
 *
 */
void RTC_Init(void)
{
  I2C_Init(); //����I/O�ڼ��ж�
  I2C_Start();   //I2C��������
  
  //����i2c��Ӧ
  /*uint8 i=0;I2C_SendByte(0xA2);    //����д����
  i=I2C_WaitAck();
  printf("%d",i);*/
  //
  
  I2C_SendByte(RTC_WR_ADD);    //����д����
  I2C_WaitAck(); //�ȴ�I2C���߻ظ�
  I2C_SendByte(RTC_CON_REG_ADD);  //ѡ���ַ0x00����״̬�Ĵ���1��֮���д��ַ�Զ���1
  I2C_WaitAck();
  I2C_SendByte(RTC_ON_DAT);    //����STOP=0��оƬʱ������
  I2C_WaitAck();
  I2C_SendByte(RTC_IRQ_ON_DAT);  //�򿪱����жϺͶ�ʱ���жϣ����巽ʽ
  I2C_WaitAck();
  I2C_Start();   //I2C��������
  I2C_SendByte(RTC_WR_ADD);  //����д����
  I2C_WaitAck();
  I2C_SendByte(RTC_ALARM_REG_ADD);  //ѡ���ַ0x09���ӱ����Ĵ���
  I2C_WaitAck();
  I2C_SendByte(RTC_ALARM_OFF_DAT);  //�رշ��ӱ���
  I2C_WaitAck();
  I2C_SendByte(RTC_ALARM_OFF_DAT);  //�ر�Сʱ����
  I2C_WaitAck();
  I2C_SendByte(RTC_ALARM_OFF_DAT);  //�ر��ձ���
  I2C_WaitAck();
  I2C_SendByte(RTC_ALARM_OFF_DAT);  //�ر����ڱ���
  I2C_WaitAck();
  I2C_SendByte(RTC_OUT_OFF_DAT);    //����CLKOUT�������Ϊ���迹
  I2C_WaitAck();
  
  
  
  
  
  
  /*���������ͨ������pcf8563�ڲ���ʱ��Ƶ�ʺͼ���ֵ���ı����ж�Ƶ�ʣ�Ĭ��Ϊ1s*/
  
#if ((SENSOR_ID!=S_WIND15) && (SENSOR_ID!=S_WIND10)) || (W_MODE==W_INT)   //���Ƿ紫������紫����ʹ���ڲ���ʱ������ʱRTCʹ��Ĭ���ж�Ƶ��
  
  I2C_SendByte(RTC_TIM_ON_DAT);     //��ʱ����Ч��Ƶ����Ϊ1Hz
  I2C_WaitAck();
  I2C_SendByte(RTC_TIM_COUNT_DAT);  //��ʱ����������������ֵ��Ϊ1��Ƶ��1Hzʱÿ1s������ʱ�ж�
  I2C_WaitAck();
  
#else  //�紫����ʹ���ⲿ��ʱ�����в���������pcf8563 250ms���һ���ж�
  
  I2C_SendByte(RTC_TIM_ON_64HZ);     //��ʱ��Ƶ����Ϊ64Hz
  I2C_WaitAck();
  I2C_SendByte(16);  //��ʱ����������������ֵ��Ϊ16��Ƶ��64Hzʱÿ0.25s������ʱ�ж�
  I2C_WaitAck();
  
#endif
  
  /**/
  
  
  
  
  
  
  I2C_Stop();   //I2C����ֹͣ
}





/*
bool RTC_WriteTime(unsigned char *time)  //�趨ʱ��
{
	I2C_Start();
	I2C_SendByte(RTC_WR_ADD);  //����д����
	I2C_WaitAck();
	I2C_SendByte(0x02);       //ѡ����Ĵ�����ַ0x02
	I2C_WaitAck();
	I2C_SendByte(*(time+6));  //д���루0x02��
	I2C_WaitAck();
	I2C_SendByte(*(time+5));  //д��֣�0x03��
	I2C_WaitAck();
	I2C_SendByte(*(time+4));  //д��ʱ��0x04��
	I2C_WaitAck();
	I2C_SendByte(*(time+3));  //д���գ�0x05��
	I2C_WaitAck();
	I2C_SendByte(*(time+2));  //д�����ڣ�0x06��
	I2C_WaitAck();
	I2C_SendByte(*(time+1));  //д���£�0x07��
	I2C_WaitAck();
	I2C_SendByte((*time));    //д���꣨0x08��
	I2C_WaitAck();
	I2C_Stop();
	return TRUE;
}

//bool WriteData(unsigned char address,unsigned char wdata)
//{
//	I2C_Start();
//	I2C_SendByte(RTC_WR_ADD);
//	I2C_WaitAck();
//	I2C_SendByte(address);
//	I2C_WaitAck();
//	I2C_SendByte(wdata);
//	I2C_WaitAck();
//	mdelay(5);
//	I2C_Stop();
//	return TRUE;
//}

//unsigned char ReadData(unsigned char address)
//{
//	unsigned char rdata;
//	I2C_Start();
//	I2C_SendByte(RTC_WR_ADD);
//	I2C_WaitAck();
//	I2C_SendByte(address);
//	I2C_WaitAck();
//	I2C_Start();
//	I2C_SendByte(RTC_RD_ADD);
//	I2C_WaitAck();
//	rdata=I2C_ReceiveByte();
//	I2C_Stop();
//	return(rdata);
//}
*/




//���趨��ַ��ʼ��ȡָ����Ŀ������
bool ReadData_s(unsigned char address,unsigned char count,unsigned char *buf)
{

	I2C_Start();
	I2C_SendByte(RTC_WR_ADD);  //����д����
	I2C_WaitAck();
	I2C_SendByte(address);   //�趨��ʼ��ַ
	I2C_WaitAck();
	I2C_Start();
	I2C_SendByte(RTC_RD_ADD); //���Ͷ����������֮ǰҪ���� д����+��ʼ��ַ ��ϣ�
	I2C_WaitAck();
	while(count)
	{
		*buf=I2C_ReceiveByte();
		if(count==1)
		{
			I2C_NoAck();
		}
		else I2C_Ack();
		buf++;
		count--;
	}
	I2C_Stop();
	return TRUE;
}

/******************************************************************************
 *
 *                ��ȡʱ��
 *
 */
uint8 RTC_GetTime(RTC_TIME_TYPE *time)
{
  uint8 timebuf[7];
  ReadData_s(0x02,7,timebuf);  //�ӵ�ַ0x02����Ĵ�������ʼ��ȡ7��ֵ����0x02-0x08������Ϊ�룬�֣�ʱ���գ����ڣ��£���
  //����������ֵ���м�飬������ȷ���ж���
  timebuf[0]&=0x7F; //��
  timebuf[1]&=0x7F; //��
  timebuf[2]&=0x3F; //ʱ
  timebuf[3]&=0x3F; //��
  timebuf[4]&=0x07; //����
  timebuf[5]&=0x1F; //��
  
  //���ʱ�䷶Χ�ĺ�����
  //ʱ���룺0x00-0x59
  if((timebuf[0]>0x59)||((timebuf[0]&0x0F)>0x09))
  {
          timebuf[0]=0x59;
  }
  if((timebuf[1]>0x59)||((timebuf[1]&0x0F)>0x09))
  {
          timebuf[1]=0x59;
  }
  if((timebuf[2]>0x23)||((timebuf[2]&0x0F)>0x09))
  {
          timebuf[2]=0x23;
  }
  //�գ�0x01-0x31
  if((timebuf[3]>0x31)||((timebuf[3]&0x0F)>0x09)||(timebuf[3]==0x00))
  {
          timebuf[3]=0x31;
  }
  //���ڣ�0x00-0x06
  if(timebuf[4]>0x06)
  {
          timebuf[4]=0x06;
  }
  //�£�0x01-0x12
  if((timebuf[5]>0x12)||((timebuf[5]&0x0F)>0x09)||(timebuf[5]==0x00))
  {
          timebuf[5]=0x12;
  }
  //�꣺0x00-0x99
  if((timebuf[6]>0x99)||((timebuf[6]&0x0F)>0x09))
  {
          timebuf[6]=0x99;
  }
  //����ʱ��ṹ��
  time->year=timebuf[6];
  time->month=timebuf[5];
  time->week=timebuf[4];
  time->day=timebuf[3];
  time->hour=timebuf[2];
  time->min=timebuf[1];
  time->sec=timebuf[0];
  return TRUE;
}


/******************************************************************************
 *
 *                ����ʱ��
 *
 */
static uint8 rtc_cnt=0;
uint8 RTC_SetTime(RTC_TIME_TYPE *time)
{
  I2C_Start(); //I2C��������
  I2C_SendByte(RTC_WR_ADD);  //����д����
  I2C_WaitAck();
  I2C_SendByte(0x02);        //ѡ����Ĵ�����ַ0x02
  I2C_WaitAck();
  I2C_SendByte(time->sec);   //д���루0x02��
  I2C_WaitAck();
  I2C_SendByte(time->min);   //д��֣�0x03��
  I2C_WaitAck();
  I2C_SendByte(time->hour);  //д��ʱ��0x04��
  I2C_WaitAck();
  I2C_SendByte(time->day);   //д���գ�0x05��
  I2C_WaitAck();
  I2C_SendByte(time->week);  //д�����ڣ�0x06��
  I2C_WaitAck();
  I2C_SendByte(time->month); //д���£�0x07��
  I2C_WaitAck();
  I2C_SendByte(time->year);  //д���꣨0x08��
  I2C_WaitAck();
  I2C_Stop(); //I2C����ֹͣ
  return TRUE;
}

//RTC�жϣ�P0���ж�
HAL_ISR_FUNCTION(halRtcPort0Isr,P0INT_VECTOR)
{
  HAL_ENTER_ISR();  //�����ж�
  
#if ((SENSOR_ID!=S_WIND15) && (SENSOR_ID!=S_WIND10)) || (W_MODE==W_INT)   //���Ƿ紫������紫����ʹ���ڲ���ʱ������ʱRTCΪ1���ж�

  osal_set_event(SampleApp_TaskID,SAMPLEAPP_1S_EVT); //ΪSampleApp��������1s��ʱ�¼�
  
  
  
#else  //�紫����ʹ���ⲿ��ʱ�����в�����RTC 250ms�ж�һ�Σ�������ټ���ֵ������250ms��ʱ�¼���ÿ4������1s��ʱ�¼�
  
  ws.wscnt_ms=T4_CNT;  //����250ms�ļ���ֵ
  T4_CNT=0;
 
    
  osal_set_event(SampleApp_TaskID,SAMPLEAPP_250MS_EVT);  //����250ms��ʱ�¼�
  
  
  rtc_cnt++;
  if(rtc_cnt>3)
  {
    rtc_cnt=0;
    osal_set_event(SampleApp_TaskID,SAMPLEAPP_1S_EVT); //����1s��ʱ�¼�
  }
    
#endif
  
  
  P0IFG=0; //���P0���������жϱ�־
  P0IF=0;  //���P0���жϱ�־
  CLEAR_SLEEP_MODE();  //���˯��״̬�������
  HAL_EXIT_ISR();  //�˳��ж�
}


#endif /* HAL_RTC */




/******************************************************************************
 *
 *                ����ʱ����غ���
 *
 */



/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.
* Assumes input in normal date format, i.e. 1980-12-31 23:59:59
* => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
*
* [For the Julian calendar (which was used in Russia before 1917,
* Britain & colonies before 1752, anywhere else before 1582,
* and is still in use by some communities) leave out the
* -year/100+year/400 terms, and add 10.]
*
* This algorithm was first published by Gauss (I think).
*
* WARNING: this function will overflow on 2106-02-07 06:28:16 on
* machines where long is 32-bit! (However, as time_t is signed, we
* will already get problems at other places on 2038-01-19 03:14:08)
*/
//�����1970��1��1��0ʱ������ʱ�侭��������
unsigned long
l_mktime(const unsigned int year0, const unsigned int mon0,
const unsigned int day, const unsigned int hour,
const unsigned int min, const unsigned int sec)
{
	unsigned long mon = mon0, year = year0;

	/* 1..12 -> 11,12,1..10 */
	if (0 >= (long)(mon -= 2)) {
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}

	return ((((unsigned long)
		(year / 4 - year / 100 + year / 400 + 367 * mon / 12 + day) +
		year * 365 - 719499
		) * 24 + hour /* now have hours */
		) * 60 + min /* now have minutes */
		) * 60 + sec; /* finally seconds */
}




//�ж�һ������Ƿ�Ϊ���꣬�Ǿͷ���1�����Ǿͷ���0
inline int isLeapYear(int year)
{
	return((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

const unsigned short DayOfMon[][13]=
{
	/* Normal years.  */
	{ 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	/* Leap years.  */
	{ 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

//��һ������ʱ������1����
void AddaMinute(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
	char leapYear = isLeapYear(*year);  //�Ƿ�����
	*minute += 1;  //����1����
	if (*minute > 59)
	{
		*minute = 0;
		*hour += 1;
		if (*hour > 23)
		{
			*hour = 0;
			*day += 1;
			if (*day > DayOfMon[leapYear][*month])  //���ڵ�������������
			{
				*day = 1;
				*month += 1;
				if (*month > 12)  //����
				{
					*month = 1;
					*year += 1;
				}
			}
		}
	}
}

//��һ������ʱ������5����
void Add5Minute(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
	for (int i = 0; i < 5; i++)
	{
		AddaMinute(year, month, day, hour, minute, second);
	}
}

//��һ������ʱ������1Сʱ
void AddaHour(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
	char leapYear = isLeapYear(*year);  //�Ƿ�����
	*hour += 1;  //����1Сʱ
	if (*hour > 23)
	{
		*hour = 0;
		*day += 1;
		if (*day > DayOfMon[leapYear][*month])  //���ڵ�������������
		{
			*day = 1;
			*month += 1;
			if (*month > 12)  //����
			{
				*month = 1;
				*year += 1;
			}
		}
	}
}





/**************************************************************************************************
**************************************************************************************************/




