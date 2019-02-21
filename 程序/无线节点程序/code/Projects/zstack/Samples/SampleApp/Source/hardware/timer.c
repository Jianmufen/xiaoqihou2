#include "timer.h"


#if SENSOR_ID==S_RF_W  //����������ʹ��232���գ�����Ҫ����������
RAINFALL_TYPE rf_w;
#endif

#if (defined HAL_COUNTER) && (HAL_COUNTER == TRUE)  //���������أ��紫������������������Ҫ��


#if (SENSOR_ID==S_WIND15) || (SENSOR_ID==S_WIND10)  //�紫����  //1.5m��10m�紫����

//WSCNT_TYPE wscnt; //���ټ���ֵ
WS_TYPE ws;  //����
WD_TYPE wd;  //����
uint8 ms_cnt=0;   //ÿ250ms������

#endif


#if SENSOR_ID==S_RF  //��������
  RAINFALL_TYPE rf;  //����
#endif /*����*/









uint16 T4_CNT,T4_CNT2;  //������ֵ

//�ڲ���������
uint32 GraytoDecimal(uint32 x);





/******************************************************************************
 *
 *               ��ʼ������
 *
 */







/*��������T4���벶���ΪP2_0�ⲿ�ж�*/

//ʹ��P2_0�жϽ��м�����������PM2ģʽ�»���
void halCounterInit(void)
{
  //����P2_0Ϊ�жϼ�������
  P2SEL&=~(1<<0);  //P2_0��Ϊͨ��IO
  P2DIR&=~(1<<0); //P2_0��Ϊ����
  P0INP&=~(1<<0);  //P2_0������
  P2INP&=~(1<<7);  //P2������
  
  //�ⲿ�ж�����
  P2IEN|=1<<0;  //P2_0�ж�ʹ��
  //PICTL|=1<<3;  //P2���½��ش���
  PICTL&=~(1<<3);  //P2�������ش���
  IEN2|=1<<1;   //P2���ж�ʹ��
  P2IFG=0x00;   //�жϱ�־��ʼΪ0
  EA=1;   //�����ж�
}

//�ⲿ�жϼ����жϣ�P2���ж�
HAL_ISR_FUNCTION(halCounterIsr,P2INT_VECTOR)
{
  HAL_ENTER_ISR();  //�����ж�
  

  T4_CNT++;  //�����ۼ�
  T4_CNT2++; 
  
  
  P2IFG=0; //���P2���������жϱ�־
  P2IF=0;  //���P2���жϱ�־
  CLEAR_SLEEP_MODE();  //���˯��״̬�������
  HAL_EXIT_ISR();  //�˳��ж�
}


/****/






void halTimer4CapInit(void)  //T4ͨ��0���벶��(P2_0)
{
  PERCFG|=1<<4;  //T4ʹ�ñ���λ��2
  P2SEL|=1<<0;  //P2_0��Ϊ����IO 
  //P2SEL&=~(1<<0);  //P2_0��Ϊͨ��IO
  P2DIR&=~(1<<0); //P2_0��Ϊ����
  P0INP&=~(1<<0);  //P2_0������
  P2INP&=~(1<<7);  //P2������
  T4CCTL0&=~(1<<2);  //ͨ��0����ģʽ
  T4CCTL0&=~(3<<0);
  T4CCTL0|=2<<0;  //ͨ��4�½��ز���
  
  T4CCTL0|=1<<6;  //ʹ�ܲ����ж� 
  T4CTL&=~(1<<3); //�ر�����ж�
  T4IE=1;  //��T4�жϺ����ж�
  EA=1;
  T4CTL|=1<<4; //����T4��ʱ��
}


//#pragma vector=T4_VECTOR
//__interrupt void T4_ISR(void)
//���벶������ж�
HAL_ISR_FUNCTION(halTimer4Isr,T4_VECTOR)   //�ж϶���ʹ��zstack�еĺ궨�壬ÿ���ж�Ҫ����HAL_ENTER_ISR();��HAL_EXIT_ISR();
{
  HAL_ENTER_ISR();  //�����ж�
  
  T4_CNT++;  //�����ۼ�
  T4_CNT2++; 
  

  CLEAR_SLEEP_MODE();  //���˯��״̬������ӷ�����ٴν���˯��ģʽ��Ӱ�����
  HAL_EXIT_ISR();  //�˳��ж�
}


void halTimer1Init(uint16 cc0,uint8 psc)  //16λ��ʱ��T1ʹ��ģģʽ��������T1CC0ʱ�����pscȡ0-3�ֱ��Ӧ��Ƶ��1,8,32,128
{
  CLKCONCMD&=~(7<<3);  //��ʱ��ʱ��ѡ��32M
  
  T1CTL&=~(3<<0);
  T1CCTL0|=1<<2;  //ģģʽҪ����ͨ��0������Ƚ�ģʽ���ܲ�������ж�
  psc&=0x03;
  T1CTL|=psc<<2;   //���÷�Ƶϵ�� 
  T1CC0L=cc0&0xFF;
  T1CC0H=(cc0>>8)&0xFF;   //�������ֵ,Ҫ��д���8λ
  
  T1CCTL0|=1<<6;   //T1ͨ��0�ж�ʹ�ܣ�ģģʽʹ��ͨ��0����Ƚϣ�
  T1IE=1;  //��T1�жϺ����ж�
  EA=1;
  T1CTL|=2<<0;  //MODEΪģģʽ����0x0000��T1CC0�ظ�����,ͬʱ������ʱ��
}
  
void RAIN_Init(void)  //����������ʼ��
{
  //halTimer4CapInit();  //T4ʹ��ͨ��0��P2_0�����벶��������
  halCounterInit();  //ʹ��P2_0���ⲿ�жϼ���
  
  //2015.9��������������
  /*�������ʹ��P1_0,P1_2,P1_3��Ϊ�������룬ԭ����P1_1��P1_4�ճ���ΪPA���ã�244ֱ��Ӳ�����Ӹߵ�ƽ
   *�̶�ѡͨԭ�����������D0,D2,D3�������룬��������ʹ��+5V���������ж����������͵�λ
   */
  P1SEL&=~0x1F;  //P1_0-P1_4��Ϊͨ��I/O��
  P1DIR&=~0x0D; //P1_0,P1_2,P1_3Ϊ�����
  
  P2INP|=1<<6;  //P1������
  //P2INP&=~(1<<6);  //P1������
     
  /* 2016.6.28���Ķ������ֻʹ��P1.0һ�����ţ�
     ��ӹ������ӵ�P1.2�жϼ�⽵ˮ״̬����ˮ��ͨ��ʱ�������ƽ���������жϣ�
     Ҳ������ǿ�ϴ�Ϊˮ��ʱÿ���Ӽ��P1.2�Ƿ�Ϊ�ߵ�ƽ�������ʾ�н�ˮ */
  //�ⲿ�ж����ã�P1.2�½��ش���
  P1IEN|=1<<2;  //P1_2�ж�ʹ��
  PICTL|=1<<1;  //P1���½��ش���
  //PICTL&=~(1<<1); //�����ش���
  IEN2|=1<<4;   //P1���ж�ʹ��
  P1IFG=0x00;   //�жϱ�־��ʼΪ0
  EA=1;   //�����ж�
  
  /* 16.7.7ע�͵� */
  /*P1DIR|=1<<4;  //P1.4���  //ʹ�÷������ã�ʹ��PAʱ����ҪӲ��ѡͨ244оƬ��ע�͵�  
  WD_RE=1;  //ѡͨ��4λ*/
  
}


/* 2016.6.28������ӹ�����ж� */
//����������жϣ�P1���ж�
HAL_ISR_FUNCTION(halRainPort1Isr,P1INT_VECTOR)
{
  HAL_ENTER_ISR();  //�����ж�
  
  
  
  if(P1IFG&(1<<2))
  {
    
    devstate.rf_raining++;   //��ˮ�ۼ�
  }
    
    
    
  
  P1IFG=0; //���P1���������жϱ�־
  P1IF=0;  //���P1���жϱ�־
  CLEAR_SLEEP_MODE();  //���˯��״̬�������
  HAL_EXIT_ISR();  //�˳��ж�
}

/* 2016.6.28��Ӷ�ȡ��ˮ״̬��������ǿ�ϴ�ˮ��ͨ��ʱÿ���Ӽ���Ƿ��н�ˮ�������ߵ�ƽ��ʾ��ˮ��ͨ�� */
uint8 get_raining_state(void)
{
  uint8 tmp=P1&(1<<2);
  tmp>>=2;
  
  return tmp;
}

/*  */

//��ȡ��������״̬����ȡԭ�������D0,D2,D3��ֵ��Ϻ󷵻أ�D3Ϊ���λ��D0Ϊ���λ
uint8 get_rain_state(void)
{
  uint8 x=0,tmp=0;
  
  //WD_RE=1;  //ѡͨ��4λ   //�ɰ�����ã��°���Ӳ��ѡͨҪȥ��
        
  /*//��ȡ���������D0,D2,D3
  x=P1&0x0F;
  
  x&=~(1<<1);  //ȥ��D1
  tmp=(x>>1)|(x&0x01);  //ȥ��D1
  tmp&=0x07;*/
  
  /* 2016.6.28���Ķ������ֻʹ��P1.0һ�����ţ�
     ��ӹ������ӵ�P1.2�жϼ�⽵ˮ״̬����ˮ��ͨ��ʱ�������ƽ���������жϣ�
     Ҳ������ǿ�ϴ�Ϊˮ��ʱÿ���Ӽ��P1.2�Ƿ�Ϊ�ߵ�ƽ�������ʾ�н�ˮ */
  tmp=P1&0x01;
  
  return tmp;
}

//2015.9.15����������ʹ��PAģ��ͷ�ĵװ�ʱ��Ҫ�����õ����ų�ʼ����ֹӰ��PA���ӹ���
void RAIN_W_Init(void)
{
  P1SEL&=~0x1F;  //P1_0-P1_4��Ϊͨ��I/O��
  P1DIR&=~0x0D; //P1_0,P1_2,P1_3Ϊ�����    //������������ûή�ͷ��ٰ幦����������������40-50mA������������PAģ������ģ���ͨģ��Ҳ��������Ӧ���ǵװ��������
  
  //P2INP|=1<<6;  //P1������
  //P2INP&=~(1<<6);  //P1������
}


#if (SENSOR_ID==S_WIND15)||(SENSOR_ID==S_WIND10)  //1.5m��10m�紫����

//#pragma vector=T1_VECTOR
//__interrupt void T1_ISR(void)
HAL_ISR_FUNCTION(halTimer1Isr,T1_VECTOR)  //T1�жϷ���
{
  HAL_ENTER_ISR();  //�����ж�
  
  if(T1STAT&(1<<0))  //CH0
  {
    
    ws.wscnt_ms=T4_CNT;  //����250ms�ļ���ֵ
    T4_CNT=0;
 
    
    osal_set_event(SampleApp_TaskID,SAMPLEAPP_250MS_EVT);  //����250ms��ʱ�¼�

    
    

  }
  
  CLEAR_SLEEP_MODE();  //���˯��״̬
  HAL_EXIT_ISR();  //�˳��ж�
}




void WS_Init(void)  //����ٶ�ʱ����ʼ��
{
  
  /*ע�����ְ��Ƶ��������߿��ܻᱻ�����˳����������ǲ���ԭ��
    �����ʵ��������ݴ�Сʹ����Ϸ��ٲ���*/
  
  /*���벶���жϼ���*/
#if (W_MODE==W_INT)  //�ڲ�ģʽ
  halTimer1Init(62500,3);   //T1��ʱ��128��Ƶ������62500����ʱ62500*��128/32000000��=250ms
  halTimer4CapInit();  //T4ʹ��ͨ��0��P2_0�����벶��������
  
  
#else  //�ⲿģʽ�����ж�ʱ������RTC_Init()�����������ú�
  
  /*�ⲿ�жϼ�����������PM2����ʱ���Ѳ�����*/
  halCounterInit();  //ʹ��P2_0���ⲿ�жϼ���
#endif
  
  
}


#endif  //�紫����


void WD_Init(void) //����ʹ��P1_0-P1_3��Ϊ�������룬P1_4��Ϊ������ѡ��Ϊ0ʱѡͨ��4λ��ȡ��Ϊ1ʱѡͨ��4λ��ȡ
{
  P1SEL&=~0x1F;  //P1_0-P1_4��Ϊͨ��I/O��
  P1DIR&=~0x0F; //P1_0-P1_3Ϊ�����
  P1DIR|=1<<4;  //P1_4Ϊ���
  P2INP|=1<<6;  //P1������
  //P2INP&=~(1<<6);  //P1������
  
  WD_RE=0;      //ѡ���4λ
}

/******************************************************************************
 *
 *              ���ݴ�����
 *
 */


//�������
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

//������תΪʮ����
uint32 GraytoDecimal(uint32 x)
{
	unsigned long y=x;
	while(x>>=1)
	{
		y^=x;
	}
	return y;
}


//�������
/*uint16 WD_Cal(void)
{
	uint8 temp,x;
	WD_RE=1;
	WD_RE=0;  //ѡͨ������
	Onboard_wait(1);  //�ȴ�1us
	x=P1&0x7F;  //��ȡ���������
	WD_RE=1;  //�ػ�����
        
	temp=GraytoDecimal(x);  //������תΪʮ����        
	if(temp>127)
	{
		temp=127;
	}
	return WD[temp];
}*/
//�������
uint16 WD_Cal(void)  //�ȶ�ȡ��4λ���ٶ���4λ�����
{
	uint8 temp,x;
        
	WD_RE=0;  //ѡͨ��4λ
        Onboard_wait(1);  //�ȴ�1us
	x=P1&0x07;  //��ȡ���������D4-D6
        
	WD_RE=1;  //ѡͨ��4λ
	Onboard_wait(1);  //�ȴ�1us
	x=(x<<4)|(P1&0x0F);  //��ȡ���������D0-D3�����λ���Ϊ7λ

        
	temp=GraytoDecimal(x);  //������תΪʮ����        
	if(temp>127)
	{
		temp=127;
	}
	return WD[temp];
}

//ģ�������㣬�������������Ӧ�ķ���ֵ
uint16 Sim_WD_Cal(uint8 gray)
{
	uint8 temp;

        
	temp=GraytoDecimal(gray);  //������תΪʮ����        
	if(temp>127)
	{
		temp=127;
	}
	return WD[temp];
}



#endif /* HAL_COUNTER */

//��ƽ������ֵ��ʹ�õ�λʸ����
#define PI 3.14159265f
uint16 aver_wd(uint16 *data,uint16 len)
{
  if(len==0) return data[0];
  uint16 i;
  float aver_x=0,aver_y=0;  //x�����y����ķ���
  float aver=0;
  float temp;
  for(i=0;i<len;i++)
  {
    temp=(((float)data[i])*PI)/180;  //�Ƕ�תΪ����
    aver_x+=sin(temp);
    aver_y+=cos(temp);
  }
  aver_x/=len;
  aver_y/=len;  //x�����y�����ƽ������
  aver=atan2(aver_x,aver_y);  //�õ�ƽ������ֵ
  aver=(aver*180)/PI;

  //����
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

  aver+=0.5;  //��������
  if(aver_x<0)  //����������atan2()�������Ϊ[-�У���]����[-�У�0)֮���ֵ��2��ת����[�У�2��)�����Եõ�[0��2��)������Ҫ�������ֵ
  {
    aver+=360;
  }
  if(aver>=360) aver=0;
    
  
  return (uint16)(aver);
}




/**************************************************************************************************
**************************************************************************************************/


