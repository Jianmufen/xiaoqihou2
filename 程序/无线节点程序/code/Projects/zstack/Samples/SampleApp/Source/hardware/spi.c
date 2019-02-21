#include "spi.h"

//SPI1的设定和传输可以参考hal_lcd.c中的LCD控制例子，halLcd_ConfigSPI()

//SPI1初始化，波特率=((256+BAUD_M)*2^BAUD_E/2^28)*时钟频率
void SPI1_Init(uint8 BAUD_E,uint8 BAUD_M)
{
  //I/O口配置
  PERCFG|=1<<1;  //USART1(SPI1)使用位置2（位于P1口)，
                 //主模式只使用P1_5，P1_6，P1_7引脚,其中P1_5为CLK，P1_6为MOSI，P1_7为MISO
  P1SEL|=7<<5;   //配置P1_5，P1_6，P1_7为外设引脚
  //P1SEL&=~0x10;        // P1_4 is GPIO (SSN)
  //P1DIR|=0x10;         // SSN is set as output //使用P1_4作为软件SSN管理引脚
  P2SEL|=1<<5;   //USART1优先
  
  //外设配置
  U1CSR&=~(1<<7); //USART1选择SPI模式
  U1CSR&=~(1<<5); //SPI主模式
  U1UCR  = 0x80;      /* Flush and goto IDLE state. 8-N-1. */
  U1GCR|=1<<7;  //SPI空闲时SCK线电平状态，CPOL=0时SCK为低电平，CPOL=1时SCK为高电平
  U1GCR|=1<<6;  //SPI时钟相位，CPHA=0时MISO在第一个边沿采样，CPHA=1时MISO在第二个边沿采样
  U1GCR|=1<<5;  //MSB先传送
  
  //通讯速率设定
  U1GCR&=0xE0;
  U1GCR|=BAUD_E&0x1F;  //BAUD_E
  U1BAUD=BAUD_M;      //BAUD_M
 
  UTX1IF=0;   //USART1 TX中断标志初始设为0
}

//SPI速率设定，波特率=((256+BAUD_M)*2^BAUD_E/2^28)*时钟频率
void SPI1_SetSpeed(uint8 BAUD_E,uint8 BAUD_M)
{
  //通讯速率设定
  U1GCR&=0xE0;
  U1GCR|=BAUD_E&0x1F;  //BAUD_E
  U1BAUD=BAUD_M;      //BAUD_M
}

uint8 SPI1_ReadWriteByte(uint8 TxData)  //发送一个字节并读取返回数据
{
  uint16 retry=0;
  
  U1RX_BYTE=0;  //清除接收标志
  U1TX_BYTE=0;  //清除发送标志
  
  U1DBUF=TxData;  //写数据到发送缓冲区
  while(!U1TX_BYTE)  //等待传送完成
  {
    retry++;
    WDT_Feed();  //喂狗
    if(retry>=0xFFFE) return 0xFC;  //发送超时退出
  }
  U1TX_BYTE=0;  //清除发送完成标志
  
  //cc2530的UxRX_BYTE位只用作UART模式和SPI从模式，不能在SPI主模式中通过它来判断是否接收完成，
  //SPI主模式把UxBUFF变成了一个doubleBUFF，然后在发送的同时，它就开启了接收，同样使用UxTX_TYTE作为接收完成标志位，
  //因为速率是匹配的，所以发送完成的时候，接收也就完成了，因此接收时要先发送无效数据提供时钟，等待UxTX_BYTE置位发送完成后直接读取UxDBUF便可
  
/*  retry=0;
  while(!U1RX_BYTE)  //等待接收数据或回应
  {
    retry++;
    if(retry>=0xFFFE)
    {
      return 0xFD;  //接收超时退出
    }
  }*/
  
  return U1DBUF; //返回接收值
}





