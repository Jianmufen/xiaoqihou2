#ifndef __TIMER_H
#define __TIMER_H

#include "ioCC2530.h"
#include "OSAL.h"
#include "hal_board.h"
#include "OnBoard.h"
#include "SampleApp.h"
//#include "sensor.h"


#define WD_RE   P1_4     //���򻺳���Ƭѡ������Ч

extern uint16 T4_CNT,T4_CNT2;  //T4����ֵ
extern uint8 ms_cnt;  //û250ms������


void halCounterInit(void);
void halTimer4CapInit(void);
void halTimer1Init(uint16 cc0,uint8 psc);
void WS_Init(void);  //���ٳ�ʼ��
void WD_Init(void);  //�����ʼ��
void RAIN_Init(void);     //������ʼ��
void RAIN_W_Init(void);   //���س�ʼ����������
uint8 get_rain_state(void); //��ȡ��������״̬
uint8 get_raining_state(void);   //��ȡ��ˮ״̬
uint16 aver_wd(uint16 *data,uint16 len);

//���ټ���ֵ��Ӧ��
static const float WS[13]=
{
	0.0,96.0,198.0,300.0,402.0,504.0,606.0,	   /*5m/s~30m/s*/
	708.0,811.0,913.5,1016.0,1118.5,1221.0	   /*35m/s~60m/s*/
};
float WS_Cal(uint16 cnt);  //���ټ���

//����������Ӧ��
static const uint16 WD[128]=
{
	0,3,6,8,11,14,17,20,22,25,28,31,34,37,39,42,45,48,51,53,56,59,62,65,68,70,73,76,79,82,84,87,
	90,93,96,98,101,104,107,110,112,115,118,121,124,127,129,132,135,138,141,143,146,149,152,155,158,160,163,166,169,172,174,177,
	180,183,186,188,191,194,197,200,202,205,208,211,214,217,219,222,225,228,231,233,236,239,242,245,248,250,253,256,259,262,264,267,
	270,273,276,278,281,284,287,290,292,295,298,301,304,307,309,312,315,318,321,323,326,329,332,335,338,340,343,346,349,352,354,357
};
uint16 WD_Cal(void);  //�������
//ģ�������㣬�������������Ӧ�ķ���ֵ
uint16 Sim_WD_Cal(uint8 gray);

          
#endif
