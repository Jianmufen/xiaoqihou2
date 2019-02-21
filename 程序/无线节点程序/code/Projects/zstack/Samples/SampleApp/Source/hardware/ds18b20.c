#include "iocc2530.h"
#include "OnBoard.h"
#include "SampleApp.h"
#include "ds18b20.h"



void Ds18b20Delay(uint16 k);
void Ds18b20InputInitial(void);//���ö˿�Ϊ����
void Ds18b20OutputInitial(void);//���ö˿�Ϊ���
uint8 Ds18b20Initial(void);
void Ds18b20Write(uint8 infor);
uint8 Ds18b20Read(void);




//ʱ��Ƶ��Ϊ32M
void Ds18b20Delay(uint16 k)
{
   MicroWait(k);//΢����ʱ
}

void Ds18b20InputInitial(void)//���ö˿�Ϊ����
{
//  P0DIR &= 0xbf;
  P0SEL &=~(1<<7);
  P0DIR &=~(1<<7);  //P0_7
}

void Ds18b20OutputInitial(void)//���ö˿�Ϊ���
{
//   P0DIR |= 0x40;
  P0SEL &=~(1<<7);
  P0DIR |=1<<7;    //P0_7
}

//ds18b20��ʼ������λ��
//��ʼ���ɹ�����0x00��ʧ�ܷ���0x01
uint8 Ds18b20Initial(void)
{
  uint8 Status = 0x00;
  uint16 CONT_1 = 0;
  uint8 Flag_1 = ON;
  Ds18b20OutputInitial();
  Ds18b20Data = 1;
  Ds18b20Delay(260);
  Ds18b20Data = 0;
  Ds18b20Delay(750);
  Ds18b20Data = 1;
  Ds18b20InputInitial();
  while((Ds18b20Data != 0)&&(Flag_1 == ON))//�ȴ�ds18b20��Ӧ�����з�ֹ��ʱ����
  {                                        //�ȴ�Լ60ms����
    CONT_1++;
    Ds18b20Delay(10);
    if(CONT_1 > 8000)Flag_1 = OFF;
    Status = Ds18b20Data;
  }
  Ds18b20OutputInitial();
  Ds18b20Data = 1;
  Ds18b20Delay(100);
  return Status;
}

//дһ���ֽڵ�ds18b20
void Ds18b20Write(uint8 infor)
{
  uint16 i;
  Ds18b20Data = 1;
  Ds18b20OutputInitial();
  Ds18b20Delay(100);
  for(i=0;i<8;i++)
  {
    if((infor & 0x01))
    {
      Ds18b20Data = 0;
      Ds18b20Delay(6);
      Ds18b20Data = 1;
      Ds18b20Delay(50);
    }
    else
    {
      Ds18b20Data = 0;
      Ds18b20Delay(50);
      Ds18b20Data = 1;
      Ds18b20Delay(6);
    }
    infor >>= 1;
  }
  Ds18b20Data = 1;
}

//��ds18b20��ȡһ���ֽ�
uint8 Ds18b20Read(void)
{
  uint8 Value = 0x00;
  uint16 i;
  Ds18b20Data = 1;
  Ds18b20OutputInitial();
  Ds18b20Data = 1;
  Ds18b20Delay(100);
  for(i=0;i<8;i++)
  {
    Value >>= 1; 
    Ds18b20OutputInitial();
    Ds18b20Data = 0;
    Ds18b20Delay(3);
    Ds18b20Data = 1;
    Ds18b20Delay(3);
    Ds18b20InputInitial();
    Ds18b20Delay(3);
    if(Ds18b20Data == 1) Value |= 0x80;
    Ds18b20Delay(15);
  }
  return Value;
}

/*

void Ds18b20Write(uint8 infor)
{
  uint16 i;
  Ds18b20OutputInitial();
  for(i=0;i<8;i++)
  {
  if((infor & 0x01))
  {
  Ds18b20Data = 0;
  Ds18b20Delay(6);
  Ds18b20Data = 1;
  Ds18b20Delay(50);
  }
  else
  {
  Ds18b20Data = 0;
  Ds18b20Delay(50);
  Ds18b20Data = 1;
  Ds18b20Delay(6);
  }
  infor >>= 1;
  }
}

uint8 Ds18b20Read(void)
{
  uint8 Value = 0x00;
  uint16 i;
  Ds18b20OutputInitial();
  Ds18b20Data = 1;
  Ds18b20Delay(10);
  for(i=0;i<8;i++)
  {
  Value >>= 1; 
  Ds18b20OutputInitial();
  Ds18b20Data = 0;
  Ds18b20Delay(3);
  Ds18b20Data = 1;
  Ds18b20Delay(3);
  Ds18b20InputInitial();
  if(Ds18b20Data == 1) Value |= 0x80;
  Ds18b20Delay(15);
  } 
  return Value;
}
*/

 //�¶ȶ�ȡ����
int16 Temp_Get(void)
{
  uint8 TL,TH;
  uint16 temp;
  int16 ds_t=0;
 
  Ds18b20Initial();    //��ʼ��ds18b20
  Ds18b20Write(0xcc);  //����ROMָ���ʹ��64λROM���룬ֻ��һ���ӻ�ʱʹ��
  Ds18b20Write(0x44);  //�¶�ת��ָ��
    
  Ds18b20Initial();    //��ʼ��ds18b20
  Ds18b20Write(0xcc);
  Ds18b20Write(0xbe);  //��ȡת�����ָ��
  
  TL = Ds18b20Read();  //ת�������ȡ���ȶ�����λ���ٶ���λ
  TH = Ds18b20Read();
  
//  temp = ((TL >> 4)+((TH & 0x07)*16));
//  return temp;
  
  //�¶ȼ���
  if(TH>7)
  {
    TH=~TH;
    TL=~TL;
    temp=0;  //�¶�Ϊ��
  }
  else
  {
    temp=1;  //�¶�Ϊ��
  }
  ds_t=TH;  //��ȡ��8λ
  ds_t<<=8;
  ds_t+=TL; //��ȡ��8λ
  ds_t=(int16)((float)ds_t*0.625+0.5);  //�����¶�ֵ���Ŵ�10��������������
  
  if(temp)
    return ds_t;
  else
    return -ds_t;

}


