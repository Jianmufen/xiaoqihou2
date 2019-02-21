#include "iocc2530.h"
#include "OnBoard.h"
#include "SampleApp.h"
#include "ds18b20.h"



void Ds18b20Delay(uint16 k);
void Ds18b20InputInitial(void);//设置端口为输入
void Ds18b20OutputInitial(void);//设置端口为输出
uint8 Ds18b20Initial(void);
void Ds18b20Write(uint8 infor);
uint8 Ds18b20Read(void);




//时钟频率为32M
void Ds18b20Delay(uint16 k)
{
   MicroWait(k);//微秒延时
}

void Ds18b20InputInitial(void)//设置端口为输入
{
//  P0DIR &= 0xbf;
  P0SEL &=~(1<<7);
  P0DIR &=~(1<<7);  //P0_7
}

void Ds18b20OutputInitial(void)//设置端口为输出
{
//   P0DIR |= 0x40;
  P0SEL &=~(1<<7);
  P0DIR |=1<<7;    //P0_7
}

//ds18b20初始化（复位）
//初始化成功返回0x00，失败返回0x01
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
  while((Ds18b20Data != 0)&&(Flag_1 == ON))//等待ds18b20响应，具有防止超时功能
  {                                        //等待约60ms左右
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

//写一个字节到ds18b20
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

//从ds18b20读取一个字节
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

 //温度读取函数
int16 Temp_Get(void)
{
  uint8 TL,TH;
  uint16 temp;
  int16 ds_t=0;
 
  Ds18b20Initial();    //初始化ds18b20
  Ds18b20Write(0xcc);  //忽略ROM指令，不使用64位ROM编码，只有一个从机时使用
  Ds18b20Write(0x44);  //温度转换指令
    
  Ds18b20Initial();    //初始化ds18b20
  Ds18b20Write(0xcc);
  Ds18b20Write(0xbe);  //读取转换结果指令
  
  TL = Ds18b20Read();  //转换结果读取，先读到低位，再读高位
  TH = Ds18b20Read();
  
//  temp = ((TL >> 4)+((TH & 0x07)*16));
//  return temp;
  
  //温度计算
  if(TH>7)
  {
    TH=~TH;
    TL=~TL;
    temp=0;  //温度为负
  }
  else
  {
    temp=1;  //温度为正
  }
  ds_t=TH;  //获取高8位
  ds_t<<=8;
  ds_t+=TL; //获取低8位
  ds_t=(int16)((float)ds_t*0.625+0.5);  //计算温度值，放大10倍后四舍五入存放
  
  if(temp)
    return ds_t;
  else
    return -ds_t;

}


