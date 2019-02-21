#include "rtc.h"

#if (defined HAL_RTC) && (HAL_RTC == TRUE)

RTC_TIME_TYPE sys_time;  //保存当前系统时间
RTC_TIME_TYPE res_time;  //用于保存某一事件前的时间



//内部函数声明

/*bool RTC_WriteTime(unsigned char *time) ;  //未使用，改用时间结构体读写时间*/




/******************************************************************************
 *
 *                初始化I2C总线I/O口和中断
 *
 */
void I2C_Init(void)  //注意模拟IIC时SSDA即是输出口也是输入口，要注意改变其方向，否则无法读取数据
{ 
  P0SEL&=~(1<<4); //P0_4和P0_5作为通用输出口（串口0的SS和C引脚不用时这两个引脚可做通用口）
  P0SEL&=~(1<<5);
  P0DIR|=3<<4;  //通用输出
  
  //设置P0_1为RTC中断输入引脚
  P0SEL&=~(1<<1); //P0_1设为通用I/O口
  P0DIR&=~(1<<1); //输入
  P0INP&=~(1<<1); //P0_1上下拉输入
  P2INP&=~(1<<5); //上拉 
  //P2INP|=1<<5;  //下拉
  
  //外部中断配置
  P0IEN|=1<<1;  //P0_1中断使能
  PICTL|=0x01;  //P0口下降沿触发
  //PICTL&=~0x01; //上升沿触发
  IEN1|=1<<5;   //P0口中断使能
  P0IFG=0x00;   //中断标志初始为0
  EA=1;   //开总中断
  //MicroWait(100);
}

/******************************************************************************
 *
 *                初始化PCF8563
 *
 */
void RTC_Init(void)
{
  I2C_Init(); //设置I/O口及中断
  I2C_Start();   //I2C总线启动
  
  //测试i2c响应
  /*uint8 i=0;I2C_SendByte(0xA2);    //发送写命令
  i=I2C_WaitAck();
  printf("%d",i);*/
  //
  
  I2C_SendByte(RTC_WR_ADD);    //发送写命令
  I2C_WaitAck(); //等待I2C总线回复
  I2C_SendByte(RTC_CON_REG_ADD);  //选择地址0x00控制状态寄存器1，之后读写地址自动增1
  I2C_WaitAck();
  I2C_SendByte(RTC_ON_DAT);    //设置STOP=0，芯片时钟运行
  I2C_WaitAck();
  I2C_SendByte(RTC_IRQ_ON_DAT);  //打开报警中断和定时器中断，脉冲方式
  I2C_WaitAck();
  I2C_Start();   //I2C总线启动
  I2C_SendByte(RTC_WR_ADD);  //发送写命令
  I2C_WaitAck();
  I2C_SendByte(RTC_ALARM_REG_ADD);  //选择地址0x09分钟报警寄存器
  I2C_WaitAck();
  I2C_SendByte(RTC_ALARM_OFF_DAT);  //关闭分钟报警
  I2C_WaitAck();
  I2C_SendByte(RTC_ALARM_OFF_DAT);  //关闭小时报警
  I2C_WaitAck();
  I2C_SendByte(RTC_ALARM_OFF_DAT);  //关闭日报警
  I2C_WaitAck();
  I2C_SendByte(RTC_ALARM_OFF_DAT);  //关闭星期报警
  I2C_WaitAck();
  I2C_SendByte(RTC_OUT_OFF_DAT);    //禁用CLKOUT输出并设为高阻抗
  I2C_WaitAck();
  
  
  
  
  
  
  /*在这里可以通过设置pcf8563内部定时器频率和计数值来改变其中断频率，默认为1s*/
  
#if ((SENSOR_ID!=S_WIND15) && (SENSOR_ID!=S_WIND10)) || (W_MODE==W_INT)   //不是风传感器或风传感器使用内部定时器测量时RTC使用默认中断频率
  
  I2C_SendByte(RTC_TIM_ON_DAT);     //定时器有效，频率设为1Hz
  I2C_WaitAck();
  I2C_SendByte(RTC_TIM_COUNT_DAT);  //定时器（倒计数）计数值设为1，频率1Hz时每1s产生到时中断
  I2C_WaitAck();
  
#else  //风传感器使用外部定时器进行测量，配置pcf8563 250ms输出一次中断
  
  I2C_SendByte(RTC_TIM_ON_64HZ);     //定时器频率设为64Hz
  I2C_WaitAck();
  I2C_SendByte(16);  //定时器（倒计数）计数值设为16，频率64Hz时每0.25s产生到时中断
  I2C_WaitAck();
  
#endif
  
  /**/
  
  
  
  
  
  
  I2C_Stop();   //I2C总线停止
}





/*
bool RTC_WriteTime(unsigned char *time)  //设定时间
{
	I2C_Start();
	I2C_SendByte(RTC_WR_ADD);  //发送写命令
	I2C_WaitAck();
	I2C_SendByte(0x02);       //选择秒寄存器地址0x02
	I2C_WaitAck();
	I2C_SendByte(*(time+6));  //写入秒（0x02）
	I2C_WaitAck();
	I2C_SendByte(*(time+5));  //写入分（0x03）
	I2C_WaitAck();
	I2C_SendByte(*(time+4));  //写入时（0x04）
	I2C_WaitAck();
	I2C_SendByte(*(time+3));  //写入日（0x05）
	I2C_WaitAck();
	I2C_SendByte(*(time+2));  //写入星期（0x06）
	I2C_WaitAck();
	I2C_SendByte(*(time+1));  //写入月（0x07）
	I2C_WaitAck();
	I2C_SendByte((*time));    //写入年（0x08）
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




//从设定地址开始读取指定数目个数据
bool ReadData_s(unsigned char address,unsigned char count,unsigned char *buf)
{

	I2C_Start();
	I2C_SendByte(RTC_WR_ADD);  //发送写命令
	I2C_WaitAck();
	I2C_SendByte(address);   //设定起始地址
	I2C_WaitAck();
	I2C_Start();
	I2C_SendByte(RTC_RD_ADD); //发送读命令（读命令之前要发送 写命令+起始地址 组合）
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
 *                获取时间
 *
 */
uint8 RTC_GetTime(RTC_TIME_TYPE *time)
{
  uint8 timebuf[7];
  ReadData_s(0x02,7,timebuf);  //从地址0x02（秒寄存器）开始读取7个值，即0x02-0x08，依次为秒，分，时，日，星期，月，年
  //对所读到的值进行检查，若不正确进行订正
  timebuf[0]&=0x7F; //秒
  timebuf[1]&=0x7F; //分
  timebuf[2]&=0x3F; //时
  timebuf[3]&=0x3F; //日
  timebuf[4]&=0x07; //星期
  timebuf[5]&=0x1F; //月
  
  //检查时间范围的合理性
  //时分秒：0x00-0x59
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
  //日：0x01-0x31
  if((timebuf[3]>0x31)||((timebuf[3]&0x0F)>0x09)||(timebuf[3]==0x00))
  {
          timebuf[3]=0x31;
  }
  //星期：0x00-0x06
  if(timebuf[4]>0x06)
  {
          timebuf[4]=0x06;
  }
  //月：0x01-0x12
  if((timebuf[5]>0x12)||((timebuf[5]&0x0F)>0x09)||(timebuf[5]==0x00))
  {
          timebuf[5]=0x12;
  }
  //年：0x00-0x99
  if((timebuf[6]>0x99)||((timebuf[6]&0x0F)>0x09))
  {
          timebuf[6]=0x99;
  }
  //存入时间结构中
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
 *                设置时间
 *
 */
static uint8 rtc_cnt=0;
uint8 RTC_SetTime(RTC_TIME_TYPE *time)
{
  I2C_Start(); //I2C总线启动
  I2C_SendByte(RTC_WR_ADD);  //发送写命令
  I2C_WaitAck();
  I2C_SendByte(0x02);        //选择秒寄存器地址0x02
  I2C_WaitAck();
  I2C_SendByte(time->sec);   //写入秒（0x02）
  I2C_WaitAck();
  I2C_SendByte(time->min);   //写入分（0x03）
  I2C_WaitAck();
  I2C_SendByte(time->hour);  //写入时（0x04）
  I2C_WaitAck();
  I2C_SendByte(time->day);   //写入日（0x05）
  I2C_WaitAck();
  I2C_SendByte(time->week);  //写入星期（0x06）
  I2C_WaitAck();
  I2C_SendByte(time->month); //写入月（0x07）
  I2C_WaitAck();
  I2C_SendByte(time->year);  //写入年（0x08）
  I2C_WaitAck();
  I2C_Stop(); //I2C总线停止
  return TRUE;
}

//RTC中断，P0口中断
HAL_ISR_FUNCTION(halRtcPort0Isr,P0INT_VECTOR)
{
  HAL_ENTER_ISR();  //进入中断
  
#if ((SENSOR_ID!=S_WIND15) && (SENSOR_ID!=S_WIND10)) || (W_MODE==W_INT)   //不是风传感器或风传感器使用内部定时器测量时RTC为1秒中断

  osal_set_event(SampleApp_TaskID,SAMPLEAPP_1S_EVT); //为SampleApp任务设置1s到时事件
  
  
  
#else  //风传感器使用外部定时器进行测量，RTC 250ms中断一次，保存风速计数值并设置250ms到时事件，每4次设置1s到时事件
  
  ws.wscnt_ms=T4_CNT;  //保存250ms的计数值
  T4_CNT=0;
 
    
  osal_set_event(SampleApp_TaskID,SAMPLEAPP_250MS_EVT);  //设置250ms到时事件
  
  
  rtc_cnt++;
  if(rtc_cnt>3)
  {
    rtc_cnt=0;
    osal_set_event(SampleApp_TaskID,SAMPLEAPP_1S_EVT); //设置1s到时事件
  }
    
#endif
  
  
  P0IFG=0; //清除P0所有引脚中断标志
  P0IF=0;  //清除P0口中断标志
  CLEAR_SLEEP_MODE();  //清除睡眠状态，必须加
  HAL_EXIT_ISR();  //退出中断
}


#endif /* HAL_RTC */




/******************************************************************************
 *
 *                其他时间相关函数
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
//计算从1970年1月1日0时到给定时间经过的秒数
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




//判断一个年份是否为闰年，是就返回1，不是就返回0
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

//将一个日期时间增加1分钟
void AddaMinute(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
	char leapYear = isLeapYear(*year);  //是否闰年
	*minute += 1;  //增加1分钟
	if (*minute > 59)
	{
		*minute = 0;
		*hour += 1;
		if (*hour > 23)
		{
			*hour = 0;
			*day += 1;
			if (*day > DayOfMon[leapYear][*month])  //大于当月天数，跨月
			{
				*day = 1;
				*month += 1;
				if (*month > 12)  //跨年
				{
					*month = 1;
					*year += 1;
				}
			}
		}
	}
}

//将一个日期时间增加5分钟
void Add5Minute(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
	for (int i = 0; i < 5; i++)
	{
		AddaMinute(year, month, day, hour, minute, second);
	}
}

//将一个日期时间增加1小时
void AddaHour(int *year, int *month, int *day, int *hour, int *minute, int *second)
{
	char leapYear = isLeapYear(*year);  //是否闰年
	*hour += 1;  //增加1小时
	if (*hour > 23)
	{
		*hour = 0;
		*day += 1;
		if (*day > DayOfMon[leapYear][*month])  //大于当月天数，跨月
		{
			*day = 1;
			*month += 1;
			if (*month > 12)  //跨年
			{
				*month = 1;
				*year += 1;
			}
		}
	}
}





/**************************************************************************************************
**************************************************************************************************/




