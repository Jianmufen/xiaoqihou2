#ifndef __24C1024_H
#define __24C1024_H

#include "i2c.h"



#define WADDR 0xAC       //24c1024д��ַ���ɵ�ַ��A1��A2Ӳ�����߾�����
#define PAGELENGTH 0x100 //24c1024ҳ����Ϊ256�ֽ�


/*24c1024��ַ����*/
#define EEP_BASE_ADDR 0x0000   //24c1024��д����ַ
#define EEP_ADDR_128  (EEP_BASE_ADDR+128)   //128�ֽڴ�
#define EEP_ADDR_256  (EEP_BASE_ADDR+256)   //256�ֽڴ�
#define EEP_ADDR_384  (EEP_BASE_ADDR+384)   //384�ֽڴ�
#define EEP_ADDR_512  (EEP_BASE_ADDR+512)   //512�ֽڴ�
#define EEP_ADDR_640  (EEP_BASE_ADDR+640)   //640�ֽڴ�
#define EEP_ADDR_768  (EEP_BASE_ADDR+768)   //768�ֽڴ�
#define EEP_ADDR_896  (EEP_BASE_ADDR+896)   //896�ֽڴ�
#define EEP_ADDR_1024 (EEP_BASE_ADDR+1024)  //1024�ֽڴ�



/*ʹ�õ��ĵ�ַ����*/
#define TEMP_CAL_ADDR  (EEP_BASE_ADDR)   //�¶�����ֵ�洢��ַ
#define BAUD_ADDR      (TEMP_CAL_ADDR+sizeof(CAL_TYPE))  //���ڲ����ʴ洢��ַ  /*����*/
#define LG_LT_ADDR     (BAUD_ADDR+sizeof(uint8))         //���侭γ�ȴ洢��ַ

//�ʿز����洢��ַ
#define QC_1ST_ADDR    (EEP_ADDR_128)    //��һ���ʿز����洢��ַ
#define QC_2ND_ADDR    (QC_1ST_ADDR+sizeof(QC_TYPE))    //�ڶ����ʿز����洢��ַ
#define QC_3RD_ADDR    (QC_2ND_ADDR+sizeof(QC_TYPE))    //�������ʿز����洢��ַ
#define QC_4TH_ADDR    (QC_3RD_ADDR+sizeof(QC_TYPE))    //���ĸ��ʿز����洢��ַ
#define QC_5TH_ADDR    (QC_4TH_ADDR+sizeof(QC_TYPE))    //������ʿز����洢��ַ

//CRУ�������洢��ַ�����£����º�ʪ���õ�
#define CR_1ST_ADDR    (EEP_ADDR_256)
#define CR_2ND_ADDR    (CR_1ST_ADDR+sizeof(Quad_Cal_Coef_t))
#define CR_3RD_ADDR    (CR_2ND_ADDR+sizeof(Quad_Cal_Coef_t))
#define CR_4TH_ADDR    (CR_3RD_ADDR+sizeof(Quad_Cal_Coef_t))
#define CR_5TH_ADDR    (CR_4TH_ADDR+sizeof(Quad_Cal_Coef_t))

/* 2015.12.3����豸״ֵ̬��Χ���� */
//���·�Χ�洢��ַ
#define BT_ADDR        (CR_5TH_ADDR+sizeof(Quad_Cal_Coef_t))
//��ص�ѹ��Χ�洢��ַ
#define BV_ADDR        (BT_ADDR+sizeof(DeviceStateRangeTypeDef))






bool EEP_ReadBytes(unsigned char *pBuffer,unsigned char length,unsigned short ReadAddress);  //��24c1024һ����ַ��ʼ��ȡlength���ֽ�
bool EEP_BuffWrite(unsigned char* pBuffer,unsigned char length,unsigned short WriteAddress);  //��24c1024һ����ַ��ʼд��length���ֽ�
bool EEP_WriteByte(unsigned char SendByte,unsigned short WriteAddress);  //��24c1024һ����ַ��д��һ���ֽ�


#endif
