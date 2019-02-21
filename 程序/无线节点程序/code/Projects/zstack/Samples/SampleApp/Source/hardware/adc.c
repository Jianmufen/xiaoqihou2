#include "adc.h"
#include "spi.h"

uint8 adc_state = START;  //AD����״̬
    
uint8 adc_done = FALSE;  //AD������ɱ�־

//�ڲ���������
void ADC_Reset(void);
void ADC_WriteByte(uint8 cmd);
uint8 ADC_ReadByte(void);
void ADC_DisSelect(void);
void ADC_Select(void);
void ADC_SpeedSet(void);


/******************************************************************************
 *
 *               ��ʼ������
 *
 */
void ADC_Reset()  //��λAD7705
{
  ADC_RST=0;  
  asm("NOP");
  asm("NOP");
  asm("NOP");
  ADC_RST=1;
  MicroWait(5);
}
 
void ADC_WriteByte(uint8 cmd)  //��AD7705дһ���ֽ����������
{
  SPI1_ReadWriteByte(cmd);  //ʹ��SPI1дAD7705
}

uint8 ADC_ReadByte(void)
{
  return SPI1_ReadWriteByte(0xFF); //SPI1д��ָ���ȡAD7705
}

//ȡ��ѡ��,�ͷ�SPI����
void ADC_DisSelect(void)
{
	ADC_CS=1;
 	SPI1_ReadWriteByte(0xff);//�ṩ�����8��ʱ��
}

//ѡ��ad7705
void ADC_Select(void)
{
	ADC_CS=0;
        SPI1_ReadWriteByte(0xff);//�ṩ�����8��ʱ��
}

//ad7705Ĭ��ͨ�������趨
void ADC_SpeedSet(void)
{
  SPI1_SetSpeed(15,0);  //1M
}

/******************************************************************************
 *
 *  AD��ʼ������ʼ��SPI1������Ƭѡ�͸�λ���ţ�
 *  ��λAD7705�����ò���ʱ�ӣ������ϵ���У׼�Ȳ���
 *
 */
void ADC_Init(void)  //AD7705��ʼ����������ʼ��SPI1�͸�λI/O��������
{
  SPI1_Init(15,0);  //SPI1��ʼ��������1M
  
  P1SEL&=~(1<<0);   //P1_0ͨ���������Ϊ���ƹ�������
  P1DIR|=1<<0;
  ADC_CV=1;  //�򿪹���
  
  P1SEL&=~(7<<1);  //P1_1-P1_3ͨ���������Ϊ4051��ͨ��ѡ��
  P1DIR|=7<<1;
  CH_SEL(0); //4051��ʼѡ��ͨ��0
  
  P1SEL&=~(1<<4);  //P1_4����Ϊͨ��������ܣ�����AD7705��λ��������
  P1DIR|=1<<4;
  ADC_RST=1; //P1_4��ʼΪ1������λ
  
  P2DIR|=3<<1;  //P2_1��P2_2����Ϊͨ�����������P2_1��ΪAD7705Ƭѡ
  ADC_CS=1;  //��ѡͨAD7705
  
  /*ADC_Reset();        //��λAD7705
  ADC_WriteByte(ADC_CLKWR_REG);  //����дʱ�ӼĴ�������
  ADC_WriteByte(ADC_RATE);   //ʱ������
  ADC_WriteByte(ADC_SETWR_REG);  //����д���üĴ�������
  ADC_WriteByte(ADC_CONTROL);  //���ò�����������У׼��������*/
}


/******************************************************************************
 *
 *               ��ȡͨ����16λת�����
 *
 */
//uint16 ADC_ReadData(uint8 channel)
//{
//  uint8 i,data;
//  uint16 result=0;
//  
//  ADC_SpeedSet(); //����AD7705ͨ������
//  ADC_Select();  //ѡ��AD7705
//  
//  for(i=0;i<200;i++)    //�ȴ�ת�����
//  {
//    ADC_WriteByte(ADC_STATE_REG|channel);  //��ͨ�żĴ����鿴DRDYλ����Ϊ0��ʾת����ɣ����Զ�ȡת��������
//    data=ADC_ReadByte();
//    if(!(data&0x80))
//      break;  //DRDYΪ0��ʾת�����
//    else
//    {
//      MicroWait(1500);  //��ʱ1.5ms
//      WDT_Feed(); //ι������ֹ��ȡʱ�����
//      //HalUARTWrite(0,"w",strlen("w"));
//    }
//  }
//  ADC_WriteByte(ADC_DATAR_REG|channel);   //��16λ���ݼĴ������ת�����
//  data=ADC_ReadByte();
//  result=data;
//  result<<=8;     //MSB first
//  data=ADC_ReadByte();
//  result|=data;
//  
//  ADC_DisSelect();  //ȡ��Ƭѡ������adת���ڼ�ʹ��spi�ϵ�������������SD����
//  return result;
//}

uint16 ADC_ReadData(uint8 channel)
{
  uint8 i,data;
  uint16 result=0;
  
  ADC_SpeedSet(); //����AD7705ͨ������
  ADC_Select();  //ѡ��AD7705
  
  for(i=0;i<200;i++)    //�ȴ�ת�����
  {
    ADC_WriteByte(ADC_STATE_REG|channel);  //��ͨ�żĴ����鿴DRDYλ����Ϊ0��ʾת����ɣ����Զ�ȡת��������
    data=ADC_ReadByte();
    if(!(data&0x80))
      break;  //DRDYΪ0��ʾת�����
    else
    {
      MicroWait(1500);  //��ʱ1.5ms
      WDT_Feed();  //ι������ֹ��ȡʱ�����
      //HalUARTWrite(0,"w",strlen("w"));
    }
  }
  ADC_WriteByte(ADC_DATAR_REG|channel);   //��16λ���ݼĴ������ת�����
  data=ADC_ReadByte();
  result=data;
  result<<=8;     //MSB first
  data=ADC_ReadByte();
  result|=data;
  
  ADC_DisSelect();  //ȡ��Ƭѡ������adת���ڼ�ʹ��spi�ϵ�������������SD����
  return result;
}
/******************************************************************************
 *
 *  ����һ��ADת����
 *  switch_ch-4051����ͨ��ѡ��0-7��
 *  channel-AD7705����ͨ����0��1����gain-���β����Ŵ���Ϊ 2^gain��buffer-ΪTRUEʹ�û�����
 *
 */
//void ADC_MeasureStart(uint8 switch_ch,uint8 channel,uint8 gain,uint8 buffer)
//{
//  //ADC_CV=1;  //�򿪹���
//  ADC_SpeedSet(); //����AD7705ͨ������
//  ADC_Select();  //ѡ��AD7705
//  
//  CH_SEL(switch_ch);  //����ͨ��ѡ��
//  //MicroWait(10000);  //�ȴ�4051�ȶ�
//  //WDT_Feed(); //ι��
//  ADC_Reset();        //��λAD7705
//  ADC_WriteByte(ADC_CLKWR_REG|channel);  //����дʱ�ӼĴ�����������ò���ͨ��
//  ADC_WriteByte(ADC_RATE);   //ʱ������
//  ADC_WriteByte(ADC_SETWR_REG|channel);  //����д���üĴ�������
//  ADC_WriteByte(ADC_CONTROL|(gain<<3)|(buffer<<1));  //���ò�����������У׼�����棬�Ƿ���Ҫ������
//  
//  ADC_DisSelect(); //ȡ��Ƭѡ������adת���ڼ�ʹ��spi�ϵ�������������SD����
//}

void ADC_MeasureStart(uint8 switch_ch,uint8 channel,uint8 gain,uint8 buffer)
{
  ADC_CV=1;  //�򿪹���
  ADC_SpeedSet(); //����AD7705ͨ������
  ADC_Select();  //ѡ��AD7705
  
  CH_SEL(switch_ch);  //����ͨ��ѡ��
  /*MicroWait(5000);*/  //�ȴ�4051�ȶ�  //ȡ������ʱ�����ɱ�׼�������ֵ����
//  MicroWait(3000);
  ADC_Reset();        //��λAD7705
  ADC_WriteByte(ADC_CLKWR_REG|channel);  //����дʱ�ӼĴ�����������ò���ͨ��
  ADC_WriteByte(ADC_RATE);   //ʱ������
  ADC_WriteByte(ADC_SETWR_REG|channel);  //����д���üĴ�������
  ADC_WriteByte(ADC_CONTROL|(gain<<3)|(buffer<<1));  //���ò�����������У׼�����棬�Ƿ���Ҫ������
  
  ADC_DisSelect(); //ȡ��Ƭѡ������adת���ڼ�ʹ��spi�ϵ�������������SD����
}



/******************************************************************************
 *
 *               �������������㲿��
 *
 */
//�������ֵ,��ѹ��λΪmv
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

//����PT100�¶�ֵ
float CalculateTemp(float fr)
{
	float fTem;
	float fL;
	float fH;
        float cal=0;  //����ֵ
	int iTem;
	uint8 cbottom,ctop;
	uint8 i;
        
        cal=temp_cal_table[0]/100;
	if(fr<RTD_TAB_PT100[0]+cal)   //�������ֵ
	{
		return -60.0;
	}
        cal=temp_cal_table[14]/100;
	if(fr>RTD_TAB_PT100[14]+cal)   //�������ֵ
	{
		return 80.0;
	}
	cbottom=0;
	ctop=14;
	for(i=7;(ctop-cbottom)!=1;)
	{
                cal=temp_cal_table[i]/100;  //�������ֵ
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
//����DHC3���͵��¶�ֵ
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
//��������ˮ��ֵ
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

