#define uint unsigned int
#define uchar unsigned char

/** PCF8563所有操作函数 **/
unsigned char bcd_to_hex(unsigned char bcd_data);                                //bcd 转 hex
unsigned char hex_to_bcd(unsigned char hex_data);                                //hex 转 bcd
void writeData(unsigned char address,unsigned char mdata);                       //向PCF8563写入一个字节数据
unsigned char ReadData(unsigned char address);                                   //从PCF8563读出一个字节数据
//void ReadData1(unsigned char address,unsigned char count,unsigned char * buff);  //从PCF8563读出多个字节数据
void PCF8563_Read(uchar *ptr);        //读入时间到内部缓冲区
void PCF8563_Gettime(uchar *get);     //获取RTC时间
void PCF8563_Settime(unsigned char *set);     //设置RCT时间

