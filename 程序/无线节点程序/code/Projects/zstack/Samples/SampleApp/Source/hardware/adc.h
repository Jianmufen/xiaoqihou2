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



#define ADC_RST P1_4  //AD7705��λ����
#define ADC_CS  P2_1  //AD7705Ƭѡ����
#define ADC_CV  P1_0  //AD7705�����������

#define VREF  (1.25)  //AD7705�ο���ѹ
#define VREF_IN (3.3) //�ڲ�adc�ο���Դ

#define CH_SEL(ch)     st( P1_1=(ch)&(1<<0); P1_2=((ch)&(1<<1))>>1; P1_3=((ch)&(1<<2))>>2;)  //4051��ģ����4052ͨ��ѡ��ʹ��P1_1,P1_2,P1_3��4052��ʹ��P1_3��
#define CH_SEL_CNT(ch) st( P0_6=(ch)&(1<<0); P2_1=((ch)&(1<<1))>>1;)  //���ְ��4052ͨ��ѡ��ʹ��P0_6��P2_1

//AD7705���ò���
#define ADC_CHN0 0x00         //ͨ��0
#define ADC_CHN1 0x01         //ͨ��1
#define ADC_STATE_REG 0x08    //��ȡͨ�żĴ�����״ֵ̬
#define ADC_CLKWR_REG 0x20    //ʱ�ӼĴ���
#define ADC_SETWR_REG 0x10    //���üĴ���
#define ADC_DATAR_REG 0x38    //�����ݼĴ���
#define ADC_RATE 0x0C         //ʱ������
#define ADC_CONTROL 0x44      //��У׼�������Թ���



//AD����״̬
typedef enum
{
  START=0,  //��ʼ����
  READY1,   //������ɿ��Զ�ȡ
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
extern uint8 adc_state;  //AD����״̬
    
extern uint8 adc_done;  //AD������ɱ�־


void ADC_Init(void);
void ADC_MeasureStart(uint8 switch_ch,uint8 channel,uint8 gain,uint8 buffer); //����һ��ת��
uint16 ADC_ReadData(uint8 channel);  //��ȡͨ����16λת�����

float CalculateRadiation(float voltage,float RadCoefficient);

//�¶����嶩������
#define CAL_T0_CONSTANT        0.2

//������嶩������
#define CAL_GH          -0.5

//�¶����嶩������
//#define CAL_T0          0.25 //2017.04.09 �����ȡ������Ϊ�ڲ�flash�洢
//�¶ȶ�����
static const float temp_cal_table[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//�¶ȼ����  
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

