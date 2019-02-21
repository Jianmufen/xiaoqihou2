#include "adc.h"
#include "spi.h"

uint8 adc_state = START;  //AD测量状态
    
uint8 adc_done = FALSE;  //AD测量完成标志

//内部函数声明
void ADC_Reset(void);
void ADC_WriteByte(uint8 cmd);
uint8 ADC_ReadByte(void);
void ADC_DisSelect(void);
void ADC_Select(void);
void ADC_SpeedSet(void);


/******************************************************************************
 *
 *               初始化部分
 *
 */
void ADC_Reset()  //复位AD7705
{
  ADC_RST=0;  
  asm("NOP");
  asm("NOP");
  asm("NOP");
  ADC_RST=1;
  MicroWait(5);
}
 
void ADC_WriteByte(uint8 cmd)  //向AD7705写一个字节命令或数据
{
  SPI1_ReadWriteByte(cmd);  //使用SPI1写AD7705
}

uint8 ADC_ReadByte(void)
{
  return SPI1_ReadWriteByte(0xFF); //SPI1写空指令读取AD7705
}

//取消选择,释放SPI总线
void ADC_DisSelect(void)
{
	ADC_CS=1;
 	SPI1_ReadWriteByte(0xff);//提供额外的8个时钟
}

//选择ad7705
void ADC_Select(void)
{
	ADC_CS=0;
        SPI1_ReadWriteByte(0xff);//提供额外的8个时钟
}

//ad7705默认通信速率设定
void ADC_SpeedSet(void)
{
  SPI1_SetSpeed(15,0);  //1M
}

/******************************************************************************
 *
 *  AD初始化，初始化SPI1，配置片选和复位引脚，
 *  复位AD7705，设置采样时钟，进行上电自校准等操作
 *
 */
void ADC_Init(void)  //AD7705初始化，包括初始化SPI1和复位I/O引脚配置
{
  SPI1_Init(15,0);  //SPI1初始化，速率1M
  
  P1SEL&=~(1<<0);   //P1_0通用输出，作为控制供电引脚
  P1DIR|=1<<0;
  ADC_CV=1;  //打开供电
  
  P1SEL&=~(7<<1);  //P1_1-P1_3通用输出，作为4051的通道选择
  P1DIR|=7<<1;
  CH_SEL(0); //4051初始选择通道0
  
  P1SEL&=~(1<<4);  //P1_4配置为通用输出功能，用作AD7705复位控制引脚
  P1DIR|=1<<4;
  ADC_RST=1; //P1_4初始为1，不复位
  
  P2DIR|=3<<1;  //P2_1和P2_2设置为通用输出，其中P2_1作为AD7705片选
  ADC_CS=1;  //不选通AD7705
  
  /*ADC_Reset();        //复位AD7705
  ADC_WriteByte(ADC_CLKWR_REG);  //发送写时钟寄存器命令
  ADC_WriteByte(ADC_RATE);   //时钟配置
  ADC_WriteByte(ADC_SETWR_REG);  //发送写设置寄存器命令
  ADC_WriteByte(ADC_CONTROL);  //设置测量参数，自校准，单极性*/
}


/******************************************************************************
 *
 *               读取通道的16位转换结果
 *
 */
//uint16 ADC_ReadData(uint8 channel)
//{
//  uint8 i,data;
//  uint16 result=0;
//  
//  ADC_SpeedSet(); //设置AD7705通信速率
//  ADC_Select();  //选择AD7705
//  
//  for(i=0;i<200;i++)    //等待转换完成
//  {
//    ADC_WriteByte(ADC_STATE_REG|channel);  //读通信寄存器查看DRDY位，若为0表示转换完成，可以读取转换数据了
//    data=ADC_ReadByte();
//    if(!(data&0x80))
//      break;  //DRDY为0表示转换完成
//    else
//    {
//      MicroWait(1500);  //延时1.5ms
//      WDT_Feed(); //喂狗，防止读取时间过长
//      //HalUARTWrite(0,"w",strlen("w"));
//    }
//  }
//  ADC_WriteByte(ADC_DATAR_REG|channel);   //读16位数据寄存器获得转换结果
//  data=ADC_ReadByte();
//  result=data;
//  result<<=8;     //MSB first
//  data=ADC_ReadByte();
//  result|=data;
//  
//  ADC_DisSelect();  //取消片选，可在ad转换期间使用spi上的其它器件，如SD卡等
//  return result;
//}

uint16 ADC_ReadData(uint8 channel)
{
  uint8 i,data;
  uint16 result=0;
  
  ADC_SpeedSet(); //设置AD7705通信速率
  ADC_Select();  //选择AD7705
  
  for(i=0;i<200;i++)    //等待转换完成
  {
    ADC_WriteByte(ADC_STATE_REG|channel);  //读通信寄存器查看DRDY位，若为0表示转换完成，可以读取转换数据了
    data=ADC_ReadByte();
    if(!(data&0x80))
      break;  //DRDY为0表示转换完成
    else
    {
      MicroWait(1500);  //延时1.5ms
      WDT_Feed();  //喂狗，防止读取时间过长
      //HalUARTWrite(0,"w",strlen("w"));
    }
  }
  ADC_WriteByte(ADC_DATAR_REG|channel);   //读16位数据寄存器获得转换结果
  data=ADC_ReadByte();
  result=data;
  result<<=8;     //MSB first
  data=ADC_ReadByte();
  result|=data;
  
  ADC_DisSelect();  //取消片选，可在ad转换期间使用spi上的其它器件，如SD卡等
  return result;
}
/******************************************************************************
 *
 *  启动一次AD转换，
 *  switch_ch-4051开关通道选择（0-7）
 *  channel-AD7705测量通道（0或1），gain-本次测量放大倍数为 2^gain，buffer-为TRUE使用缓冲器
 *
 */
//void ADC_MeasureStart(uint8 switch_ch,uint8 channel,uint8 gain,uint8 buffer)
//{
//  //ADC_CV=1;  //打开供电
//  ADC_SpeedSet(); //设置AD7705通信速率
//  ADC_Select();  //选择AD7705
//  
//  CH_SEL(switch_ch);  //开关通道选择
//  //MicroWait(10000);  //等待4051稳定
//  //WDT_Feed(); //喂狗
//  ADC_Reset();        //复位AD7705
//  ADC_WriteByte(ADC_CLKWR_REG|channel);  //发送写时钟寄存器命令，并设置测量通道
//  ADC_WriteByte(ADC_RATE);   //时钟配置
//  ADC_WriteByte(ADC_SETWR_REG|channel);  //发送写设置寄存器命令
//  ADC_WriteByte(ADC_CONTROL|(gain<<3)|(buffer<<1));  //设置测量参数，自校准，增益，是否需要缓冲器
//  
//  ADC_DisSelect(); //取消片选，可在ad转换期间使用spi上的其它器件，如SD卡等
//}

void ADC_MeasureStart(uint8 switch_ch,uint8 channel,uint8 gain,uint8 buffer)
{
  ADC_CV=1;  //打开供电
  ADC_SpeedSet(); //设置AD7705通信速率
  ADC_Select();  //选择AD7705
  
  CH_SEL(switch_ch);  //开关通道选择
  /*MicroWait(5000);*/  //等待4051稳定  //取消，有时候会造成标准电阻测量值错误
//  MicroWait(3000);
  ADC_Reset();        //复位AD7705
  ADC_WriteByte(ADC_CLKWR_REG|channel);  //发送写时钟寄存器命令，并设置测量通道
  ADC_WriteByte(ADC_RATE);   //时钟配置
  ADC_WriteByte(ADC_SETWR_REG|channel);  //发送写设置寄存器命令
  ADC_WriteByte(ADC_CONTROL|(gain<<3)|(buffer<<1));  //设置测量参数，自校准，增益，是否需要缓冲器
  
  ADC_DisSelect(); //取消片选，可在ad转换期间使用spi上的其它器件，如SD卡等
}



/******************************************************************************
 *
 *               传感器测量计算部分
 *
 */
//计算辐射值,电压单位为mv
float CalculateRadiation(float voltage,float RadCoefficient)
{
  voltage*=1000;
  
  if(RadCoefficient<=0)
  {
    RadCoefficient=10;
  }
  
  voltage/=RadCoefficient;
  
  voltage=MAX(voltage,0.0);
  voltage=MIN(voltage,5000.0);
  
  return voltage;
}

//计算PT100温度值
float CalculateTemp(float fr)
{
	float fTem;
	float fL;
	float fH;
        float cal=0;  //修正值
	int iTem;
	uint8 cbottom,ctop;
	uint8 i;
        
        cal=temp_cal_table[0]/100;
	if(fr<RTD_TAB_PT100[0]+cal)   //添加修正值
	{
		return -60.0;
	}
        cal=temp_cal_table[14]/100;
	if(fr>RTD_TAB_PT100[14]+cal)   //添加修正值
	{
		return 80.0;
	}
	cbottom=0;
	ctop=14;
	for(i=7;(ctop-cbottom)!=1;)
	{
                cal=temp_cal_table[i]/100;  //添加修正值
		if(fr<RTD_TAB_PT100[i]+cal)
		{
			ctop=i;
			i=(ctop+cbottom)/2;
		}
		else if(fr>RTD_TAB_PT100[i]+cal)
		{
			cbottom=i;
			i=(ctop+cbottom)/2;
		}
		else
		{
			iTem=(unsigned int)i*10-60;
			fTem=(float)iTem;
			return fTem;
		}
	}
	iTem=(int)i*10-60;
	fL=RTD_TAB_PT100[cbottom]+temp_cal_table[cbottom]/100;
	fH=RTD_TAB_PT100[ctop]+temp_cal_table[ctop]/100;
	fTem=(((fr-fL)*10)/(fH-fL))+iTem;
	return fTem;
}

/*
//计算DHC3类型的温度值
float CalculateTemp_DHC3(float fr)
{
	float fTem;
	float fL;
	float fH;
	int iTem;
	uint8 cbottom,ctop;
	uint8 i;
	if(fr<RTD_TAB_PT100_DHC3[0])
	{
		return -60.0;
	}
	if(fr>RTD_TAB_PT100_DHC3[12])
	{
		return 60.0;
	}
	cbottom=0;
	ctop=12;
	for(i=6;(ctop-cbottom)!=1;)
	{
		if(fr<RTD_TAB_PT100_DHC3[i])
		{
			ctop=i;
			i=(ctop+cbottom)/2;
		}
		else if(fr>RTD_TAB_PT100_DHC3[i])
		{
			cbottom=i;
			i=(ctop+cbottom)/2;
		}
		else
		{
			iTem=(unsigned int)i*10-60;
			fTem=(float)iTem;
			return fTem;
		}
	}
	iTem=(int)i*10-60;
	fL=RTD_TAB_PT100_DHC3[cbottom];
	fH=RTD_TAB_PT100_DHC3[ctop];
	fTem=(((fr-fL)*10)/(fH-fL))+iTem;
	return fTem;
}
*/

/*
//计算土壤水分值
float CalculateSand(float voltage)
{
	float fTem;
	float fL;
	float fH;
	int iTem;
	uint8 cbottom,ctop;
	uint8 i;
	if(voltage<SAND[0])
	{
		return 0;
	}
	if(voltage>SAND[20])
	{
		return 100;
	}
	cbottom=0;
	ctop=20;
	for(i=10;(ctop-cbottom)!=1;)
	{
		if(voltage<SAND[i])
		{
			ctop=i;
			i=(ctop+cbottom)/2;
		}
		else if(voltage>SAND[i])
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
	fL=SAND[cbottom];
	fH=SAND[ctop];
	fTem=(((voltage-fL)*5)/(fH-fL))+iTem;
	return fTem;
}
*/











/**************************************************************************************************
**************************************************************************************************/

