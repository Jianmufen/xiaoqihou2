#include "spi.h"

//SPI1���趨�ʹ�����Բο�hal_lcd.c�е�LCD�������ӣ�halLcd_ConfigSPI()

//SPI1��ʼ����������=((256+BAUD_M)*2^BAUD_E/2^28)*ʱ��Ƶ��
void SPI1_Init(uint8 BAUD_E,uint8 BAUD_M)
{
  //I/O������
  PERCFG|=1<<1;  //USART1(SPI1)ʹ��λ��2��λ��P1��)��
                 //��ģʽֻʹ��P1_5��P1_6��P1_7����,����P1_5ΪCLK��P1_6ΪMOSI��P1_7ΪMISO
  P1SEL|=7<<5;   //����P1_5��P1_6��P1_7Ϊ��������
  //P1SEL&=~0x10;        // P1_4 is GPIO (SSN)
  //P1DIR|=0x10;         // SSN is set as output //ʹ��P1_4��Ϊ���SSN��������
  P2SEL|=1<<5;   //USART1����
  
  //��������
  U1CSR&=~(1<<7); //USART1ѡ��SPIģʽ
  U1CSR&=~(1<<5); //SPI��ģʽ
  U1UCR  = 0x80;      /* Flush and goto IDLE state. 8-N-1. */
  U1GCR|=1<<7;  //SPI����ʱSCK�ߵ�ƽ״̬��CPOL=0ʱSCKΪ�͵�ƽ��CPOL=1ʱSCKΪ�ߵ�ƽ
  U1GCR|=1<<6;  //SPIʱ����λ��CPHA=0ʱMISO�ڵ�һ�����ز�����CPHA=1ʱMISO�ڵڶ������ز���
  U1GCR|=1<<5;  //MSB�ȴ���
  
  //ͨѶ�����趨
  U1GCR&=0xE0;
  U1GCR|=BAUD_E&0x1F;  //BAUD_E
  U1BAUD=BAUD_M;      //BAUD_M
 
  UTX1IF=0;   //USART1 TX�жϱ�־��ʼ��Ϊ0
}

//SPI�����趨��������=((256+BAUD_M)*2^BAUD_E/2^28)*ʱ��Ƶ��
void SPI1_SetSpeed(uint8 BAUD_E,uint8 BAUD_M)
{
  //ͨѶ�����趨
  U1GCR&=0xE0;
  U1GCR|=BAUD_E&0x1F;  //BAUD_E
  U1BAUD=BAUD_M;      //BAUD_M
}

uint8 SPI1_ReadWriteByte(uint8 TxData)  //����һ���ֽڲ���ȡ��������
{
  uint16 retry=0;
  
  U1RX_BYTE=0;  //������ձ�־
  U1TX_BYTE=0;  //������ͱ�־
  
  U1DBUF=TxData;  //д���ݵ����ͻ�����
  while(!U1TX_BYTE)  //�ȴ��������
  {
    retry++;
    WDT_Feed();  //ι��
    if(retry>=0xFFFE) return 0xFC;  //���ͳ�ʱ�˳�
  }
  U1TX_BYTE=0;  //���������ɱ�־
  
  //cc2530��UxRX_BYTEλֻ����UARTģʽ��SPI��ģʽ��������SPI��ģʽ��ͨ�������ж��Ƿ������ɣ�
  //SPI��ģʽ��UxBUFF�����һ��doubleBUFF��Ȼ���ڷ��͵�ͬʱ�����Ϳ����˽��գ�ͬ��ʹ��UxTX_TYTE��Ϊ������ɱ�־λ��
  //��Ϊ������ƥ��ģ����Է�����ɵ�ʱ�򣬽���Ҳ������ˣ���˽���ʱҪ�ȷ�����Ч�����ṩʱ�ӣ��ȴ�UxTX_BYTE��λ������ɺ�ֱ�Ӷ�ȡUxDBUF���
  
/*  retry=0;
  while(!U1RX_BYTE)  //�ȴ��������ݻ��Ӧ
  {
    retry++;
    if(retry>=0xFFFE)
    {
      return 0xFD;  //���ճ�ʱ�˳�
    }
  }*/
  
  return U1DBUF; //���ؽ���ֵ
}





