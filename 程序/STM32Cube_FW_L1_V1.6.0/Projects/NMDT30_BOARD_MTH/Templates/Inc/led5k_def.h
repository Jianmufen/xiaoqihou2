#ifndef __LED5K_DEF_H
#define __LED5K_DEF_H


/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"
#include "main.h"

#include "led5k_conf.h"



#ifdef _WIN32  /* windowsƽ̨ */
#include "windows.h"
#define __STATIC           static
#define __STATIC_INLINE    static __inline
#define __INLINE           __inline
#endif


#ifdef __cplusplus
extern "C" {
#endif 



/* Ĭ����Ļ�������壬��led5k_conf.h��δ������ʹ��Ĭ��ֵ */
/* LED��Ļ��̬����ߴ� */
#ifndef LED_SCREEN_WIDTH
#define LED_SCREEN_WIDTH   (128)
#endif
#ifndef LED_SCREEN_HEIGHT
#define LED_SCREEN_HEIGHT  (80)
#endif


/* ������Ļ�������� */
#ifndef LED_SET_SCREEN_PARAM
#define LED_SET_SCREEN_PARAM  (ScreenParam_128_80)
#endif





//������
typedef enum __bx5k_err
{
	ERR_NO = 0, //No Error 
	ERR_OUTOFGROUP = 1, //Command Group Error 
	ERR_NOCMD = 2, //Command Not Found 
	ERR_BUSY = 3, //The Controller is busy now 
	ERR_MEMORYVOLUME = 4, //Out of the Memory Volume 
	ERR_CHECKSUM = 5, //CRC16 Checksum Error 
	ERR_FILENOTEXIST = 6, //File Not Exist 
	ERR_FLASH = 7, //Flash Access Error 
	ERR_FILE_DOWNLOAD = 8, //File Download Error 
	ERR_FILE_NAME = 9, //Filename Error 
	ERR_FILE_TYPE = 10,//File type Error 
	ERR_FILE_CRC16 = 11,//File CRC16 Error 
	ERR_FONT_NOT_EXIST = 12,//Font Library Not Exist 
	ERR_FIRMWARE_TYPE = 13,//Firmware Type Error (Check the controller type) 
	ERR_DATE_TIME_FORMAT = 14,//Date Time format error 
	ERR_FILE_EXIST = 15,//File Exist for File overwrite 
	ERR_FILE_BLOCK_NUM = 16,//File block number error 
	ERR_COMMUNICATE = 100,//ͨ��ʧ��
	ERR_PROTOCOL = 101,//Э�����ݲ���ȷ
	ERR_TIMEOUT = 102,//ͨ�ų�ʱ
	ERR_NETCLOSE = 103,//ͨ�ŶϿ�
	ERR_INVALID_HAND = 104,//��Ч���
	ERR_PARAMETER = 105,//��������
	ERR_SHOULDREPEAT = 106,//��Ҫ�ظ��ϴ����ݰ�
	ERR_FILE = 107,//��Ч�ļ�
} bx5k_err;


//����������
typedef enum __bx_5k_card_type
{
	BX_Any = 0xFE,
	BX_5K1 = 0x51,
	BX_5K2 = 0x58,
	BX_5MK2 = 0x53,
	BX_5MK1 = 0x54,
} bx_5k_card_type;


#pragma pack(push)  /* ���浱ǰ����״̬ */
#pragma pack(1)     /* �趨Ϊ1�ֽڶ��� */

/* PHY1��
* ��ͷ����(14byte) | ������(Nbyte) | ��У��(2byte)
* PHY1 ��ʵ��������ķ����������Ϊ���������Ӱ�ͷ������������ݵ�У��ֵ��
*/

/* PHY0��
* ֡ͷ(8byte) | PHY1 ������(Nbyte) | ֡β(1byte)
* PHY0 ��ΪPHY1 ����������֡ͷ��֡β������PHY1 �����ݽ���ת�壨�ο��ַ�ת�壩��
*/


//-----֡ͷ���ݸ�ʽ------
// frame header | packet header | data |
//---------------------
//��ͷ����
typedef struct __bx_5k_frame_header
{
	uint8_t header[8];
} bx_5k_frame_header;

//-----��ͷ���ݸ�ʽ------
// packet header | data |
//---------------------
//��ͷ����
typedef struct __bx_5k_packet_header
{
	uint16_t DstAddr;        //����ַ
	uint16_t SrcAddr;        //Դ��ַ
	uint8_t  Reserved1;      //����
	uint8_t  Reserved2;
	uint8_t  Reserved3;
	uint8_t  Reserved4;
	uint8_t  Reserved5;
	uint8_t  DisplayMode;    //��ʾģʽ
	uint8_t  DeviceType;     //�豸����
	uint8_t  ProtocolVersion;//Э��汾��
	uint16_t DataLen;        //�����򳤶�
} bx_5k_packet_header;

//-----��̬��ʾ����ͷ��ʽ------
// dynamic header | area data |
//---------------------
//��̬��ʾ����ͷ����
typedef struct __bx_5k_dynamic_header
{
	uint8_t  CmdGroup;      //���������
	uint8_t  Cmd;           //������
	uint8_t  Response;      //�Ƿ�Ҫ��������ظ�
	uint8_t  Reserved1;     //����
	uint8_t  Reserved2;
	uint8_t  DeleteAreaNum; //Ҫɾ���������������ʹ�ã��̶�Ϊ0��
	uint8_t  AreaNum;       //������������θ��µ�����������̶�ʹ��1����̬����
	uint16_t AreaDataLen0;  //����0 ���ݳ��ȣ�ֻʹ��1����̬����0��
} bx_5k_dynamic_header;

//-------�����ʽ------
// area header | data |
//---------------------
//��Ŀ��������
typedef struct __bx_5k_area_header
{
	uint8_t  AreaType;          //��������
	uint16_t AreaX;             //����X ���꣬���ֽ�(8 �����ص�)Ϊ��λ
	uint16_t AreaY;             //����Y ���꣬�����ص�Ϊ��λ
	uint16_t AreaWidth;         //�����ȣ����ֽ�(8 �����ص�)Ϊ��λ
	uint16_t AreaHeight;        //����߶ȣ������ص�Ϊ��λ
	uint8_t  DynamicAreaLoc;    //��̬�����ţ���̬������ͳһ��ţ���Ŵ�0 ��ʼ����
	uint8_t  Lines_sizes;       //�м��
	uint8_t  RunMode;           //��̬������ģʽ:
	//0�� ��̬������ѭ����ʾ��
	//1�� ��̬��������ʾ��ɺ�ֹ��ʾ���һҳ���ݡ�
	//2�� ��̬������ѭ����ʾ�������趨ʱ���������δ����ʱɾ����̬����Ϣ��
	uint16_t Timeout;           //��̬�����ݳ�ʱʱ�䣬��λΪ��
	uint8_t  Reserved1;         //����
	uint8_t  Reserved2;
	uint8_t  Reserved3;
	uint8_t  SingleLine;        //�Ƿ�����ʾ:0x01����������ʾ,0x02����������ʾ
	uint8_t  NewLine;           //�Ƿ��Զ�����:0x01�������Զ����У�0x02�����Զ�����
	uint8_t  DisplayMode;       //��ʾ��ʽ���䶨�����£�
	//0x01������ֹ��ʾ
	//0x02�������ٴ��
	//0x03���������ƶ�
	//0x04���������ƶ�
	//0x05���������ƶ�
	//0x06���������ƶ�
	uint8_t  ExitMode;          //�˳���ʽ
	uint8_t  Speed;             //��ʾ�ٶȣ��������£�
	//0x00�������
	//0x01����
	//����
	//0x17����
	//0x18 ��������
	uint8_t  StayTime;          //��ʾ�ؼ�ͣ��ʱ�䣬��λΪ0.5s
	uint32_t DataLen;           //���ݳ��ȣ��������У���ɫ��ת�������
} bx_5k_area_header;




#pragma pack(pop)  /* �ָ�����״̬ */






#ifdef __cplusplus
}
#endif

#endif /* __LED5K_H */