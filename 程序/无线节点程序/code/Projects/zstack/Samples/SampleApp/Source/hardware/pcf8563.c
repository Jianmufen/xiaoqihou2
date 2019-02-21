#include <ioCC2530.h>
#include <pcf8563.h>
#include <I2C.h>
#include <string.h>

uchar PCF8563_time[7];

/** PCF8563所有操作函数声明 **/
unsigned char bcd_to_hex(unsigned char bcd_data);                                //bcd 转 hex
unsigned char hex_to_bcd(unsigned char hex_data);                                //hex 转 bcd
void writeData(unsigned char address,unsigned char mdata);                       //向PCF8563写入一个字节数据
unsigned char ReadData(unsigned char address);                                   //从PCF8563读出一个字节数据
//void ReadData1(unsigned char address,unsigned char count,unsigned char * buff);  //从PCF8563读出多个字节数据
void PCF8563_Read(uchar *ptr);        //读入时间到内部缓冲区
void PCF8563_Gettime(uchar *get);     //获取RTC时间
void PCF8563_Settime(unsigned char *set);     //设置RCT时间

/***********************************************************************/
/********** 　函数名称 bcd_to_hex(unsigned char bcd_data)  *************/
/***********  功能 ： bcd 转 hex　　************************************/
/***********************************************************************/

unsigned char bcd_to_hex(unsigned char bcd_data) 
{   
  unsigned char temp;  
  temp=((bcd_data>>4)*10)+(bcd_data&0x0F);   
  return temp;    
}

/***********************************************************************/
/********** 　函数名称 bcd_to_hex(unsigned char bcd_data)  ********/
/***********  功能 ： hex 转 bcd　　************************************/
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
/*******  函数名称 writeData(unsigned char address,unsigned char mdata)  ******/
/*******  功能 ： 向PCF8563写入一个字节数据    ********************************/
/******************************************************************************/

void writeData(unsigned char address,unsigned char mdata) 
{   
  Start();  
  writebyte(0xa2); /*写命令*/ 
  writebyte(address); /*写地址*/  
  writebyte(mdata); /*写数据*/ 
  Stop();
}

/******************************************************************************/
/************ 函数名称 unsigned char ReadData(unsigned char address) **********/
/***********　功能 ： 向PCF8563读出一个字节数据    ****************************/
/******************************************************************************/

unsigned char ReadData(unsigned char address) /*单字节*/ 
{   
  unsigned char  rdata; 
  Start(); 
  writebyte(0xa2); /*写命令*/  
  writebyte(address); /*写地址*/ 
  Start();   
  writebyte(0xa3); /*读命令*/ 
  rdata=Readbyte(); 
  WriteACK(1);  
  Stop();  
  return(rdata);
}

/**********************************************************************************************************/
/************ 函数名称 ReadData1(unsigned char address,unsigned char count,unsigned char * buff) **********/
/***********　功能 ： 向PCF8563读出多个字节数据    ********************************************************/
/**********************************************************************************************************/

//void ReadData1(unsigned char address,unsigned char count,unsigned char * buff) /*多字节*/ 
//{    
//     unsigned char i; 
//     Start();  
//     writebyte(0xa2); /*写命令*/ 
//     writebyte(address); /*写地址*/  
//     Start();  
//     writebyte(0xa3); /*读命令*/  
//     for(i=0;i<count;i++) 
//     {   
//       buff[i]=Readbyte();  
//       if(i<count-1) WriteACK(0); 
//     }   
//     WriteACK(1);  
//     Stop();
//}

/****************************************************/
/************ 函数名称 PCF8563_Read()****************/
/***********　功能 ： 读入时间到内部缓冲区   ********/
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
  ptr[0]=(time[0]&0x7f); /*秒*/  
  ptr[1]=(time[1]&0x7f); /*分*/ 
  ptr[2]=(time[2]&0x3f); /*小时*/  
  ptr[3]=(time[3]&0x3f); /*日*/  
  ptr[4]=time[4]&0x07; /*星期*/ 
  ptr[5]=(time[5]&0x1f); /*月/世纪*/ 
  ptr[6]=(time[6]); /*年*/ 
}

/****************************************************/
/************ 函数名称 PCF8563_Gettime()*************/
/***********　功能 ： 获取RTC时间   *****************/
/****************************************************/

void PCF8563_Gettime(uchar *get)
{   
  PCF8563_Read(get); 
  if(get[0]==0)  
    PCF8563_Read(get); /*如果为秒=0，为防止时间变化，再读一次*/
} 

/****************************************************/
/************ 函数名称 PCF8563_Settime()*************/
/***********　功能 ： 设置RCT时间   *****************/
/****************************************************/

void PCF8563_Settime(unsigned char *set)
{   
     unsigned char i; 
     for(i=2;i<9;i++)
     { 
       writeData(i,set[i-2]);
     }   
}
