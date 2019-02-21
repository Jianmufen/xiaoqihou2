#define uint unsigned int
#define uchar unsigned char

/** PCF8563���в������� **/
unsigned char bcd_to_hex(unsigned char bcd_data);                                //bcd ת hex
unsigned char hex_to_bcd(unsigned char hex_data);                                //hex ת bcd
void writeData(unsigned char address,unsigned char mdata);                       //��PCF8563д��һ���ֽ�����
unsigned char ReadData(unsigned char address);                                   //��PCF8563����һ���ֽ�����
//void ReadData1(unsigned char address,unsigned char count,unsigned char * buff);  //��PCF8563��������ֽ�����
void PCF8563_Read(uchar *ptr);        //����ʱ�䵽�ڲ�������
void PCF8563_Gettime(uchar *get);     //��ȡRTCʱ��
void PCF8563_Settime(unsigned char *set);     //����RCTʱ��

