#include <ioCC2530.h>
#include <pcf8563.h>
#include <I2C.h>
#include <string.h>

uchar PCF8563_time[7];

/** PCF8563���в����������� **/
unsigned char bcd_to_hex(unsigned char bcd_data);                                //bcd ת hex
unsigned char hex_to_bcd(unsigned char hex_data);                                //hex ת bcd
void writeData(unsigned char address,unsigned char mdata);                       //��PCF8563д��һ���ֽ�����
unsigned char ReadData(unsigned char address);                                   //��PCF8563����һ���ֽ�����
//void ReadData1(unsigned char address,unsigned char count,unsigned char * buff);  //��PCF8563��������ֽ�����
void PCF8563_Read(uchar *ptr);        //����ʱ�䵽�ڲ�������
void PCF8563_Gettime(uchar *get);     //��ȡRTCʱ��
void PCF8563_Settime(unsigned char *set);     //����RCTʱ��

/***********************************************************************/
/********** ���������� bcd_to_hex(unsigned char bcd_data)  *************/
/***********  ���� �� bcd ת hex����************************************/
/***********************************************************************/

unsigned char bcd_to_hex(unsigned char bcd_data) 
{   
  unsigned char temp;  
  temp=((bcd_data>>4)*10)+(bcd_data&0x0F);   
  return temp;    
}

/***********************************************************************/
/********** ���������� bcd_to_hex(unsigned char bcd_data)  ********/
/***********  ���� �� hex ת bcd����************************************/
/***********************************************************************/

unsigned char hex_to_bcd(unsigned char hex_data)  
{    
  unsigned char temp;  
  unsigned char bcd_data;   
  temp=hex_data%100;    
  bcd_data=(temp/10<<4)|(temp%10);   
  return bcd_data; 
}

/******************************************************************************/
/*******  �������� writeData(unsigned char address,unsigned char mdata)  ******/
/*******  ���� �� ��PCF8563д��һ���ֽ�����    ********************************/
/******************************************************************************/

void writeData(unsigned char address,unsigned char mdata) 
{   
  Start();  
  writebyte(0xa2); /*д����*/ 
  writebyte(address); /*д��ַ*/  
  writebyte(mdata); /*д����*/ 
  Stop();
}

/******************************************************************************/
/************ �������� unsigned char ReadData(unsigned char address) **********/
/***********������ �� ��PCF8563����һ���ֽ�����    ****************************/
/******************************************************************************/

unsigned char ReadData(unsigned char address) /*���ֽ�*/ 
{   
  unsigned char  rdata; 
  Start(); 
  writebyte(0xa2); /*д����*/  
  writebyte(address); /*д��ַ*/ 
  Start();   
  writebyte(0xa3); /*������*/ 
  rdata=Readbyte(); 
  WriteACK(1);  
  Stop();  
  return(rdata);
}

/**********************************************************************************************************/
/************ �������� ReadData1(unsigned char address,unsigned char count,unsigned char * buff) **********/
/***********������ �� ��PCF8563��������ֽ�����    ********************************************************/
/**********************************************************************************************************/

//void ReadData1(unsigned char address,unsigned char count,unsigned char * buff) /*���ֽ�*/ 
//{    
//     unsigned char i; 
//     Start();  
//     writebyte(0xa2); /*д����*/ 
//     writebyte(address); /*д��ַ*/  
//     Start();  
//     writebyte(0xa3); /*������*/  
//     for(i=0;i<count;i++) 
//     {   
//       buff[i]=Readbyte();  
//       if(i<count-1) WriteACK(0); 
//     }   
//     WriteACK(1);  
//     Stop();
//}

/****************************************************/
/************ �������� PCF8563_Read()****************/
/***********������ �� ����ʱ�䵽�ڲ�������   ********/
/****************************************************/

void PCF8563_Read(uchar *ptr) 
{   
  uint i;
  unsigned char time[7];  
  for(i=2;i<=8;i++)
  {
    time[i-2]=ReadData(i);
  }
  //ReadData1(0x02,0x08,time);  
  ptr[0]=(time[0]&0x7f); /*��*/  
  ptr[1]=(time[1]&0x7f); /*��*/ 
  ptr[2]=(time[2]&0x3f); /*Сʱ*/  
  ptr[3]=(time[3]&0x3f); /*��*/  
  ptr[4]=time[4]&0x07; /*����*/ 
  ptr[5]=(time[5]&0x1f); /*��/����*/ 
  ptr[6]=(time[6]); /*��*/ 
}

/****************************************************/
/************ �������� PCF8563_Gettime()*************/
/***********������ �� ��ȡRTCʱ��   *****************/
/****************************************************/

void PCF8563_Gettime(uchar *get)
{   
  PCF8563_Read(get); 
  if(get[0]==0)  
    PCF8563_Read(get); /*���Ϊ��=0��Ϊ��ֹʱ��仯���ٶ�һ��*/
} 

/****************************************************/
/************ �������� PCF8563_Settime()*************/
/***********������ �� ����RCTʱ��   *****************/
/****************************************************/

void PCF8563_Settime(unsigned char *set)
{   
     unsigned char i; 
     for(i=2;i<9;i++)
     { 
       writeData(i,set[i-2]);
     }   
}
