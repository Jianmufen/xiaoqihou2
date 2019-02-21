#ifndef __ADC_H
#define __ADC_H

#include "ioCC2530.h"
#include "OSAL.h"
#include "hal_board.h"
#include "OnBoard.h"
#include "SampleApp.h"
#include "hal_defs.h"
#include "math.h"
#include "stdlib.h"
#include "spi.h"
#include "wdt.h"
//#include "timer.h"



#define ADC_RST P1_4  //AD7705复位引脚
#define ADC_CS  P2_1  //AD7705片选引脚
#define ADC_CV  P1_0  //AD7705供电控制引脚

#define VREF  (1.25)  //AD7705参考电压
#define VREF_IN (3.3) //内部adc参考电源

#define CH_SEL(ch)     st( P1_1=(ch)&(1<<0); P1_2=((ch)&(1<<1))>>1; P1_3=((ch)&(1<<2))>>2;)  //4051和模拟板的4052通道选择，使用P1_1,P1_2,P1_3（4052不使用P1_3）
#define CH_SEL_CNT(ch) st( P0_6=(ch)&(1<<0); P2_1=((ch)&(1<<1))>>1;)  //数字板的4052通道选择，使用P0_6和P2_1

//AD7705设置参数
#define ADC_CHN0 0x00         //通道0
#define ADC_CHN1 0x01         //通道1
#define ADC_STATE_REG 0x08    //读取通信寄存器的状态值
#define ADC_CLKWR_REG 0x20    //时钟寄存器
#define ADC_SETWR_REG 0x10    //设置寄存器
#define ADC_DATAR_REG 0x38    //读数据寄存器
#define ADC_RATE 0x0C         //时钟配置
#define ADC_CONTROL 0x44      //自校准，单极性工作



//AD测量状态
typedef enum
{
  START=0,  //开始测量
  READY1,   //测量完成可以读取
  READY2,
  READY3,
  READY4,
  READY5,
  READY6,
  READY7,
  READY8,
  READY9,
  READY10
}ADC_STATE;
extern uint8 adc_state;  //AD测量状态
    
extern uint8 adc_done;  //AD测量完成标志


void ADC_Init(void);
void ADC_MeasureStart(uint8 switch_ch,uint8 channel,uint8 gain,uint8 buffer); //启动一次转换
uint16 ADC_ReadData(uint8 channel);  //读取通道的16位转换结果

float CalculateRadiation(float voltage,float RadCoefficient);

//温度整体订正常量
#define CAL_T0_CONSTANT        0.2

//光合整体订正常量
#define CAL_GH          -0.5

//温度整体订正常量
//#define CAL_T0          0.25 //2017.04.09 杨广湘取消，改为内部flash存储
//温度订正表
static const float temp_cal_table[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//温度计算表  
static const float RTD_TAB_PT100[15]=
{
	76.33,80.31,84.27,88.22,92.16,96.09,100.00,103.90,       /*-60C~+10C*/		
	107.79,111.67,115.54,119.40,123.24,127.07,130.89		 /*+20C~+80C*/
};
float CalculateTemp(float fr);
//static const float RTD_TAB_PT100_DHC3[13]=
//{
//	76.41,80.48,84.45,88.42,92.36,96.30,100.23,104.13,       /*-60C~+10C*/		
//	108.03,111.91,115.78,119.64,123.50//,127.07,130.89		 /*+20C~+60C*/
//};
//float CalculateTemp_DHC3(float fr);



static const float SAND[21]=
{
	115.0,340.0,450.0,530.0,610.0,690.0,750.0,825.0,895.0,955.0,1005.0,		 /*0%~50%*/
	1015.0,1025.0,1035.0,1045.0,1055.0,1065.0,1070.0,1080.0,1095.0,1106.0	 /*55%~100%*/
};
float CalculateSand(float voltage);



#endif

