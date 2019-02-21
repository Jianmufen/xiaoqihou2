#include "Led5k.h"
#include "led5k_param.h"
#include "crc.h"

#include "usart.h"
#include "cmsis_os.h"



//����������ݣ���������ͷ+����ͼ������
uint32_t Led5kFillAreaData(uint8_t *Data, const struct tm *DateTime, const void *MeasureData, const uint8_t *InputText, uint32_t InputLength)
{
	bx_5k_area_header *area_header = (bx_5k_area_header *)Data;
	uint32_t count = 0;

	if (Data == NULL)
	{
		return 0;
	}

	//����������ͷ�����ͼ������
	count = Led5kFillAreaText(Data + sizeof(bx_5k_area_header), DateTime, MeasureData, InputText, InputLength);

	//����ͼ��������Ϣ����������ݰ�ͷ
	area_header->AreaType = 0;
	area_header->AreaX = 0;
	area_header->AreaY = 0;
	area_header->AreaWidth = LED_SCREEN_WIDTH / 8;
	area_header->AreaHeight = LED_SCREEN_HEIGHT;
	area_header->DynamicAreaLoc = 0;
	area_header->Lines_sizes = 0;
	area_header->RunMode = 0;
	area_header->Timeout = 2;
	area_header->Reserved1 = 0;
	area_header->Reserved2 = 0;
	area_header->Reserved3 = 0;
	area_header->SingleLine = 0x02;    //������ʾ
	area_header->NewLine = 0x02;       //�Զ�����
	area_header->DisplayMode = 0x02;   //���ٴ��
	area_header->ExitMode = 0;
	area_header->Speed = 0x00;         //�ٶ����
	area_header->StayTime = LED_STAY_TIME*2/*10*/;        //ͣ��5s  /* 16.6.14����ͣ��ʱ������� */
	area_header->DataLen = count;      //��ʾ���ݳ���

	return (count + sizeof(bx_5k_area_header));
}


//��䶯̬�������ݣ���̬����ͷ+�������ݣ�ֻʹ��һ������0��
uint32_t Led5kFillDynamicData(uint8_t *Data, const struct tm *DateTime, const void *MeasureData, const uint8_t *InputText, uint32_t InputLength)
{
	bx_5k_dynamic_header *dynamic_header = (bx_5k_dynamic_header *)Data;
	uint32_t count = 0;

	if (Data == NULL)
	{
		return 0;
	}

	//�ڶ�̬����ͷ�������������
	count = Led5kFillAreaData(Data + sizeof(bx_5k_dynamic_header), DateTime, MeasureData, InputText, InputLength);
        

	//��������������Ϣ��䶯̬�������ݰ�ͷ
	dynamic_header->CmdGroup = 0xA3;  //��̬����A3 06
	dynamic_header->Cmd = 06;
	dynamic_header->Response = 0x01;
	/*dynamic_header->Reserved1 = 0;
	dynamic_header->Reserved2 = 0;*/
	dynamic_header->Reserved1 = (count + 4) & 0xFF;  //�Դ����������������λ�÷���������ݳ��ȣ���������Ҳ��������Ա�
	dynamic_header->Reserved2 = (count + 4) >> 8;
	dynamic_header->DeleteAreaNum = 0;
	dynamic_header->AreaNum = 1;      //����led��Ļ��Ϊһ����̬����
	dynamic_header->AreaDataLen0 = count;  //�������ݳ���

	return (count + sizeof(bx_5k_dynamic_header));
}


//���PHY1�㣺��ͷ����(14byte) | ������(Nbyte) | ��У��(2byte) 
uint32_t Led5kFillPHY1Data(uint8_t *Data, const struct tm *DateTime, const void *MeasureData,const uint8_t *InputText,uint32_t InputLength)
{
	bx_5k_packet_header *packet_header = (bx_5k_packet_header *)Data;
	uint32_t count = 0;
	uint16_t crc = 0;

	if (Data == NULL)
	{
		return 0;
	}

	//�ڰ�ͷ����䶯̬����
	count = Led5kFillDynamicData(Data + sizeof(bx_5k_packet_header), DateTime, MeasureData, InputText, InputLength);

	//���ݶ�̬������Ϣ����ͷ
	packet_header->DstAddr = 0x0001;
	packet_header->SrcAddr = 0x8000;
	packet_header->Reserved1 = 0;
	packet_header->Reserved2 = 0;
	packet_header->Reserved3 = 0;
	packet_header->Reserved4 = 0;
	packet_header->Reserved5 = 0;
	packet_header->DisplayMode = 0x00;  //��ͨģʽ
	packet_header->DeviceType = BX_Any; //�豸����ͨ��
	packet_header->ProtocolVersion = 0x02;
	packet_header->DataLen = count;     //�����򳤶�

	count += sizeof(bx_5k_packet_header);
        

	//����crc16
	crc = CalcCRC(Data, count);

	//��crc16�ŵ�ĩβ
	*((uint16_t *)(Data + count)) = crc;

	return (count + 2);
}



//���Ͷ�̬����֡
static UART_HandleTypeDef *huart_led=&huart3;  /* led display uart */
int32_t SendDynamicArea(uint8_t *Data, uint32_t Length)
{
	uint32_t i = 0;
        uint8_t buf[4]={0};

        buf[0]=0xA5;
	//8�ֽ�֡ͷ0xA5
	for (i = 0; i < 8; i++)
	{
		/*printf("%02X ", 0xA5);*/
          HAL_UART_Transmit(huart_led,buf,1,1000);
	}

	//ת��������
	for (i = 0; i < Length; i++)
	{
		//��֡������0xA5����֮ת��Ϊ0xA6,0x02
		if (Data[i] == 0xA5)
		{
			/*printf("%02X %02X ", 0xA6, 0x02);*/
                  buf[0]=0xA6;
                  buf[1]=0x02;
                  HAL_UART_Transmit(huart_led,buf,2,1000);
		}

		//������0xA6����֮ת��Ϊ0xA6,0x01
		else if (Data[i] == 0xA6)
		{
			/*printf("%02X %02X ", 0xA6, 0x01);*/
                  buf[0]=0xA6;
                  buf[1]=0x01;
                  HAL_UART_Transmit(huart_led,buf,2,1000);
		}

		//��֡������0x5A����֮ת��Ϊ0x5B,0x02
		else if (Data[i] == 0x5A)
		{
			/*printf("%02X %02X ", 0x5B, 0x02);*/
                  buf[0]=0x5B;
                  buf[1]=0x02;
                  HAL_UART_Transmit(huart_led,buf,2,1000);
		}

		//������0x5B����֮ת��Ϊ0x5B,0x01
		else if (Data[i] == 0x5B)
		{
			/*printf("%02X %02X ", 0x5B, 0x01);*/
                  buf[0]=0x5B;
                  buf[1]=0x01;
                  HAL_UART_Transmit(huart_led,buf,2,1000);
		}
		else
		{
			/*printf("%02X ", Data[i]);*/
                  buf[0]=Data[i];
                  HAL_UART_Transmit(huart_led,buf,1,1000);
		}
	}

	//1�ֽ�֡β0x5A
	/*printf("%02X", 0x5A);*/
        buf[0]=0x5A;
        HAL_UART_Transmit(huart_led,buf,1,1000);

	return 0;
}




/* ����LED��Ļ���� */
//��������
void Led5kSetScreenParam(void)
{
	/*uint32_t i = 0;*/

	//���Ϳ�ʼд�����ļ�����
	/*for (i = 0; i < sizeof(StartWriteScreenParamFileCMD); i++)
	{
		printf("%02X ", StartWriteScreenParamFileCMD[i]);
	}*/
        HAL_UART_Transmit(huart_led,(uint8_t *)StartWriteScreenParamFileCMD,sizeof(StartWriteScreenParamFileCMD),1000);

	//���200ms
#ifdef _WIN32  /* windowsƽ̨ */
	Sleep(200);
#elif defined osCMSIS
        if(osKernelRunning())   /* os is running */
        {
          osDelay(200);
        }
        else
        {
          HAL_Delay(200);
        }
#else
	HAL_Delay(200);
#endif
	

	//������Ļ����LED_SET_SCREEN_PARAM
	/*for (i = 0; i < sizeof(LED_SET_SCREEN_PARAM); i++)
	{
		printf("%02X ", LED_SET_SCREEN_PARAM[i]);
	}*/
        HAL_UART_Transmit(huart_led,(uint8_t *)LED_SET_SCREEN_PARAM,sizeof(LED_SET_SCREEN_PARAM),1000);

}


