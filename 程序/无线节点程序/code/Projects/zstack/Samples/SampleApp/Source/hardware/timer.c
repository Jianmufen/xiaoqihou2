#include "timer.h"


#if SENSOR_ID==S_RF_W  //称重雨量，使用232接收，不需要开启计数器
RAINFALL_TYPE rf_w;
#endif

#if (defined HAL_COUNTER) && (HAL_COUNTER == TRUE)  //计数器开关，风传感器和雨量传感器需要打开


#if (SENSOR_ID==S_WIND15) || (SENSOR_ID==S_WIND10)  //风传感器  //1.5m和10m风传感器

//WSCNT_TYPE wscnt; //风速计数值
WS_TYPE ws;  //风速
WD_TYPE wd;  //风向
uint8 ms_cnt=0;   //每250ms计数器

#endif


#if SENSOR_ID==S_RF  //雨量计数
  RAINFALL_TYPE rf;  //雨量
#endif /*雨量*/









uint16 T4_CNT,T4_CNT2;  //计数器值

//内部函数声明
uint32 GraytoDecimal(uint32 x);





/******************************************************************************
 *
 *               初始化部分
 *
 */







/*将计数由T4输入捕获改为P2_0外部中断*/

//使用P2_0中断进行计数，可以在PM2模式下唤醒
void halCounterInit(void)
{
  //设置P2_0为中断计数引脚
  P2SEL&=~(1<<0);  //P2_0设为通用IO
  P2DIR&=~(1<<0); //P2_0设为输入
  P0INP&=~(1<<0);  //P2_0上下拉
  P2INP&=~(1<<7);  //P2口上拉
  
  //外部中断配置
  P2IEN|=1<<0;  //P2_0中断使能
  //PICTL|=1<<3;  //P2口下降沿触发
  PICTL&=~(1<<3);  //P2口上升沿触发
  IEN2|=1<<1;   //P2口中断使能
  P2IFG=0x00;   //中断标志初始为0
  EA=1;   //开总中断
}

//外部中断计数中断，P2口中断
HAL_ISR_FUNCTION(halCounterIsr,P2INT_VECTOR)
{
  HAL_ENTER_ISR();  //进入中断
  

  T4_CNT++;  //计数累计
  T4_CNT2++; 
  
  
  P2IFG=0; //清除P2所有引脚中断标志
  P2IF=0;  //清除P2口中断标志
  CLEAR_SLEEP_MODE();  //清除睡眠状态，必须加
  HAL_EXIT_ISR();  //退出中断
}


/****/






void halTimer4CapInit(void)  //T4通道0输入捕获(P2_0)
{
  PERCFG|=1<<4;  //T4使用备用位置2
  P2SEL|=1<<0;  //P2_0设为外设IO 
  //P2SEL&=~(1<<0);  //P2_0设为通用IO
  P2DIR&=~(1<<0); //P2_0设为输入
  P0INP&=~(1<<0);  //P2_0上下拉
  P2INP&=~(1<<7);  //P2口上拉
  T4CCTL0&=~(1<<2);  //通道0捕获模式
  T4CCTL0&=~(3<<0);
  T4CCTL0|=2<<0;  //通道4下降沿捕获
  
  T4CCTL0|=1<<6;  //使能捕获中断 
  T4CTL&=~(1<<3); //关闭溢出中断
  T4IE=1;  //开T4中断和总中断
  EA=1;
  T4CTL|=1<<4; //启动T4定时器
}


//#pragma vector=T4_VECTOR
//__interrupt void T4_ISR(void)
//输入捕获计数中断
HAL_ISR_FUNCTION(halTimer4Isr,T4_VECTOR)   //中断定义使用zstack中的宏定义，每个中断要加入HAL_ENTER_ISR();和HAL_EXIT_ISR();
{
  HAL_ENTER_ISR();  //进入中断
  
  T4_CNT++;  //计数累计
  T4_CNT2++; 
  

  CLEAR_SLEEP_MODE();  //清除睡眠状态，必须加否则会再次进入睡眠模式而影响计数
  HAL_EXIT_ISR();  //退出中断
}


void halTimer1Init(uint16 cc0,uint8 psc)  //16位定时器T1使用模模式，计数到T1CC0时溢出，psc取0-3分别对应分频数1,8,32,128
{
  CLKCONCMD&=~(7<<3);  //定时器时钟选择32M
  
  T1CTL&=~(3<<0);
  T1CCTL0|=1<<2;  //模模式要开启通道0的输出比较模式才能产生溢出中断
  psc&=0x03;
  T1CTL|=psc<<2;   //设置分频系数 
  T1CC0L=cc0&0xFF;
  T1CC0H=(cc0>>8)&0xFF;   //设置溢出值,要先写入低8位
  
  T1CCTL0|=1<<6;   //T1通道0中断使能（模模式使用通道0输出比较）
  T1IE=1;  //开T1中断和总中断
  EA=1;
  T1CTL|=2<<0;  //MODE为模模式，从0x0000到T1CC0重复计数,同时开启定时器
}
  
void RAIN_Init(void)  //雨量计数初始化
{
  //halTimer4CapInit();  //T4使用通道0（P2_0）输入捕获来计数
  halCounterInit();  //使用P2_0的外部中断计数
  
  //2015.9添加雨量堵塞检测
  /*堵塞检查使用P1_0,P1_2,P1_3作为并行输入，原来的P1_1和P1_4空出来为PA备用，244直接硬件连接高电平
   *固定选通原来风向输入的D0,D2,D3三个输入，三个输入使用+5V上拉，若有堵塞则拉到低电位
   */
  P1SEL&=~0x1F;  //P1_0-P1_4作为通用I/O口
  P1DIR&=~0x0D; //P1_0,P1_2,P1_3为输入口
  
  P2INP|=1<<6;  //P1口下拉
  //P2INP&=~(1<<6);  //P1口上拉
     
  /* 2016.6.28更改堵塞检测只使用P1.0一个引脚；
     添加光电管连接到P1.2中断检测降水状态，有水滴通过时会产生电平跳变引起中断；
     也可在雨强较大为水流时每秒钟检测P1.2是否为高电平，是则表示有降水 */
  //外部中断配置，P1.2下降沿触发
  P1IEN|=1<<2;  //P1_2中断使能
  PICTL|=1<<1;  //P1口下降沿触发
  //PICTL&=~(1<<1); //上升沿触发
  IEN2|=1<<4;   //P1口中断使能
  P1IFG=0x00;   //中断标志初始为0
  EA=1;   //开总中断
  
  /* 16.7.7注释掉 */
  /*P1DIR|=1<<4;  //P1.4输出  //使用风板测试用，使用PA时最终要硬件选通244芯片并注释掉  
  WD_RE=1;  //选通低4位*/
  
}


/* 2016.6.28雨量添加光电检测中断 */
//雨量光电检测中断，P1口中断
HAL_ISR_FUNCTION(halRainPort1Isr,P1INT_VECTOR)
{
  HAL_ENTER_ISR();  //进入中断
  
  
  
  if(P1IFG&(1<<2))
  {
    
    devstate.rf_raining++;   //降水累计
  }
    
    
    
  
  P1IFG=0; //清除P1所有引脚中断标志
  P1IF=0;  //清除P1口中断标志
  CLEAR_SLEEP_MODE();  //清除睡眠状态，必须加
  HAL_EXIT_ISR();  //退出中断
}

/* 2016.6.28添加读取降水状态，用于雨强较大水流通过时每秒钟检测是否有降水，持续高电平表示有水流通过 */
uint8 get_raining_state(void)
{
  uint8 tmp=P1&(1<<2);
  tmp>>=2;
  
  return tmp;
}

/*  */

//获取雨量堵塞状态，读取原来风向的D0,D2,D3的值组合后返回，D3为最高位，D0为最低位
uint8 get_rain_state(void)
{
  uint8 x=0,tmp=0;
  
  //WD_RE=1;  //选通低4位   //旧版测试用，新版由硬件选通要去掉
        
  /*//读取风向格雷码D0,D2,D3
  x=P1&0x0F;
  
  x&=~(1<<1);  //去除D1
  tmp=(x>>1)|(x&0x01);  //去除D1
  tmp&=0x07;*/
  
  /* 2016.6.28更改堵塞检测只使用P1.0一个引脚；
     添加光电管连接到P1.2中断检测降水状态，有水滴通过时会产生电平跳变引起中断；
     也可在雨强较大为水流时每秒钟检测P1.2是否为高电平，是则表示有降水 */
  tmp=P1&0x01;
  
  return tmp;
}

//2015.9.15，称重雨量使用PA模块和风的底板时需要将不用的引脚初始化防止影响PA增加功耗
void RAIN_W_Init(void)
{
  P1SEL&=~0x1F;  //P1_0-P1_4作为通用I/O口
  P1DIR&=~0x0D; //P1_0,P1_2,P1_3为输入口    //经测试这个配置会降低风速板功耗至正常，否则有40-50mA，并不是由于PA模块引起的，普通模块也会这样，应该是底板输入造成
  
  //P2INP|=1<<6;  //P1口下拉
  //P2INP&=~(1<<6);  //P1口上拉
}


#if (SENSOR_ID==S_WIND15)||(SENSOR_ID==S_WIND10)  //1.5m和10m风传感器

//#pragma vector=T1_VECTOR
//__interrupt void T1_ISR(void)
HAL_ISR_FUNCTION(halTimer1Isr,T1_VECTOR)  //T1中断服务
{
  HAL_ENTER_ISR();  //进入中断
  
  if(T1STAT&(1<<0))  //CH0
  {
    
    ws.wscnt_ms=T4_CNT;  //保存250ms的计数值
    T4_CNT=0;
 
    
    osal_set_event(SampleApp_TaskID,SAMPLEAPP_250MS_EVT);  //设置250ms到时事件

    
    

  }
  
  CLEAR_SLEEP_MODE();  //清除睡眠状态
  HAL_EXIT_ISR();  //退出中断
}




void WS_Init(void)  //测风速定时器初始化
{
  
  /*注意数字板的频率输入过高可能会被电容滤除掉，并不是测量原因，
    可以适当调整电容大小使其符合风速采样*/
  
  /*输入捕获中断计数*/
#if (W_MODE==W_INT)  //内部模式
  halTimer1Init(62500,3);   //T1定时器128分频，计数62500，定时62500*（128/32000000）=250ms
  halTimer4CapInit();  //T4使用通道0（P2_0）输入捕获来计数
  
  
#else  //外部模式，其中定时部分在RTC_Init()函数中已配置好
  
  /*外部中断计数，可以在PM2休眠时唤醒并计数*/
  halCounterInit();  //使用P2_0的外部中断计数
#endif
  
  
}


#endif  //风传感器


void WD_Init(void) //风向使用P1_0-P1_3作为并行输入，P1_4作为缓冲器选择，为0时选通高4位读取，为1时选通低4位读取
{
  P1SEL&=~0x1F;  //P1_0-P1_4作为通用I/O口
  P1DIR&=~0x0F; //P1_0-P1_3为输入口
  P1DIR|=1<<4;  //P1_4为输出
  P2INP|=1<<6;  //P1口下拉
  //P2INP&=~(1<<6);  //P1口上拉
  
  WD_RE=0;      //选择高4位
}

/******************************************************************************
 *
 *              数据处理部分
 *
 */


//计算风速
float WS_Cal(uint16 cnt)
{
	uint8 i,cbottom,ctop;
	float fTem,iTem,fH,fL;
	if(cnt<=WS[0]) return 0;
	else if(cnt>=WS[12]) return 60;
	else if(cnt<=1) return 0.3;
	else if((cnt>=1.0)&&(cnt<4.0))
	{
		iTem=0.3;
		fL=1.0;
		fH=4.0;
		fTem=(((cnt-fL)*0.2)/(fH-fL))+iTem;
		return fTem; 
	}
	else if((cnt>=4.0)&&(cnt<14.0))
	{
		iTem=0.5;
		fL=4.0;
		fH=14.0;
		fTem=(((cnt-fL)*0.5)/(fH-fL))+iTem;
		return fTem; 
	}
	else if((cnt>=14.0)&&(cnt<25.0))
	{
		iTem=1.0;
		fL=14.0;
		fH=25.0;
		fTem=(((cnt-fL)*0.5)/(fH-fL))+iTem;
		return fTem; 
	}
	else if((cnt>=25.0)&&(cnt<35.0))
	{
		iTem=1.5;
		fL=25.0;
		fH=35.0;
		fTem=(((cnt-fL)*0.5)/(fH-fL))+iTem;
		return fTem; 
	}
	else if((cnt>=35.0)&&(cnt<96.0))
	{
		iTem=2.0;
		fL=35.0;
		fH=96.0;
		fTem=(((cnt-fL)*3.0)/(fH-fL))+iTem;
		return fTem; 
	}
	else
	{
		cbottom=0;
		ctop=12;
		for(i=6;(ctop-cbottom)!=1;)
		{
			if(cnt<WS[i])
			{
				ctop=i;
				i=(ctop+cbottom)/2;
			}
			else if(cnt>WS[i])
			{
				cbottom=i;
				i=(ctop+cbottom)/2;
			}
			else
			{
				iTem=(unsigned int)i*5;
				fTem=(float)iTem;
				return fTem;
			}
		}
		iTem=(unsigned int)i*5;
		fL=WS[cbottom];
		fH=WS[ctop];
		fTem=(((cnt-fL)*5)/(fH-fL))+iTem;
		return fTem;
	}
}

//格雷码转为十进制
uint32 GraytoDecimal(uint32 x)
{
	unsigned long y=x;
	while(x>>=1)
	{
		y^=x;
	}
	return y;
}


//风向计算
/*uint16 WD_Cal(void)
{
	uint8 temp,x;
	WD_RE=1;
	WD_RE=0;  //选通缓冲器
	Onboard_wait(1);  //等待1us
	x=P1&0x7F;  //读取风向格雷码
	WD_RE=1;  //关缓冲器
        
	temp=GraytoDecimal(x);  //格雷码转为十进制        
	if(temp>127)
	{
		temp=127;
	}
	return WD[temp];
}*/
//风向计算
uint16 WD_Cal(void)  //先读取高4位，再读低4位后计算
{
	uint8 temp,x;
        
	WD_RE=0;  //选通高4位
        Onboard_wait(1);  //等待1us
	x=P1&0x07;  //读取风向格雷码D4-D6
        
	WD_RE=1;  //选通低4位
	Onboard_wait(1);  //等待1us
	x=(x<<4)|(P1&0x0F);  //读取风向格雷码D0-D3并与高位组合为7位

        
	temp=GraytoDecimal(x);  //格雷码转为十进制        
	if(temp>127)
	{
		temp=127;
	}
	return WD[temp];
}

//模拟风向计算，输入格雷码计算对应的风向值
uint16 Sim_WD_Cal(uint8 gray)
{
	uint8 temp;

        
	temp=GraytoDecimal(gray);  //格雷码转为十进制        
	if(temp>127)
	{
		temp=127;
	}
	return WD[temp];
}



#endif /* HAL_COUNTER */

//求平均风向值，使用单位矢量法
#define PI 3.14159265f
uint16 aver_wd(uint16 *data,uint16 len)
{
  if(len==0) return data[0];
  uint16 i;
  float aver_x=0,aver_y=0;  //x方向和y方向的分量
  float aver=0;
  float temp;
  for(i=0;i<len;i++)
  {
    temp=(((float)data[i])*PI)/180;  //角度转为弧度
    aver_x+=sin(temp);
    aver_y+=cos(temp);
  }
  aver_x/=len;
  aver_y/=len;  //x方向和y方向的平均分量
  aver=atan2(aver_x,aver_y);  //得到平均弧度值
  aver=(aver*180)/PI;

  //测试
  if(test)
  {
    //uint8 buf[64];
    printf("aver_x:%f.\r\n",aver_x);
    //HalUARTWrite(0,buf,strlen(buf));
    printf("aver_y:%f.\r\n",aver_y);
    //HalUARTWrite(0,buf,strlen(buf));
    printf("aver:%f.\r\n",aver);
    //HalUARTWrite(0,buf,strlen(buf));
  }

  aver+=0.5;  //四舍五入
  if(aver_x<0)  //风向修正，atan2()函数结果为[-π，π]，将[-π，0)之间的值加2π转换到[π，2π)，可以得到[0，2π)的满足要求的修正值
  {
    aver+=360;
  }
  if(aver>=360) aver=0;
    
  
  return (uint16)(aver);
}




/**************************************************************************************************
**************************************************************************************************/


