#include "Led5k.h"



__STATIC_INLINE uint32_t FillText(uint8_t *Data, const struct tm *DateTime, const void *MeasureData);
__STATIC_INLINE uint32_t FillText_QingDao_JiaoZhou(uint8_t *Data, const struct tm *DateTime, const void *MeasureData);



//�������ͼ������
uint32_t Led5kFillAreaText(uint8_t *Data, const struct tm *DateTime, const void *MeasureData, const uint8_t *InputText, uint32_t InputLength)
{
	uint32_t count = 0;

	if (Data == NULL)
	{
		return 0;
	}

	if (InputText&&InputLength)  //��ʾ�ⲿ��������ݣ����ڵ���
	{
		memcpy(Data, InputText, InputLength);

		return InputLength;
	}


	//��ʾĬ���ı�
        return FillText(Data, DateTime, MeasureData);

}



/**
  * @}
  */

/**
  * @}
  */




//���ݲ�ͬ��Ļ�����ʾ�ı�
__STATIC_INLINE uint32_t FillText(uint8_t *Data, const struct tm *DateTime, const void *MeasureData)
{
	if (Data == NULL)
	{
		return 0;
	}

#if (LED_LOCATION==LED_QingDao_JiaoZhou)     /* �ൺ���ݣ�128*80 */

	return FillText_QingDao_JiaoZhou(Data, DateTime, MeasureData);

#else  //Ĭ����Ļ��ʾ����
        uint32_t count=0;
	int16_t temp_value = -1888, hum_value = 100, radg_value = 1357, radp_value = 2468,co2_value=1234,wd_value=121,ws_value=230,rf_value=133,sm_value=11;

	if (DateTime)
	{

	}

	if (MeasureData)
	{

	}

	count += sprintf(Data + count, "����:    %5.1f��", (float)temp_value / 100);
	count += sprintf(Data + count, "ʪ��:    %3d%%RH", hum_value);
	count += sprintf(Data + count, "����:   %4dW/m2", radg_value);
	count += sprintf(Data + count, "���:   %4dumol", radp_value);
	count += sprintf(Data + count, "10cm����:%5.1f��", (float)temp_value / 100);

	return count;

#endif   /* LED_LOCATION */
}


/**
  * @}
  */

/**
  * @}
  */


/* �ൺ���ݣ�128*80��ʾ�ı� */
__STATIC_INLINE uint32_t FillText_QingDao_JiaoZhou(uint8_t *Data, const struct tm *DateTime, const void *MeasureData)
{
	uint32_t count = 0;
	int16_t temp_value = -11, hum_value = 11, radg_value = 135, radp_value = 246,co2_value=1234,wd_value=36,ws_value=6523,rf_value=133,sm_value=11;

	if (Data == NULL)
	{
		return 0;
	}

	if (DateTime)
	{
#ifdef _WIN32
		//tm_yearΪ��1900�������,tm_mon��0��ʼ
		count += sprintf(Data + count, "%04d-%02d-%02d %02d:%02d",
			DateTime->tm_year+1900, DateTime->tm_mon+1, DateTime->tm_mday,
			DateTime->tm_hour, DateTime->tm_min);
#else
		//tm_yearֱ����Ϊ���,tm_mon��1��ʼ
		count += sprintf(Data + count, "%04d-%02d-%02d %02d:%02d",
			DateTime->tm_year , DateTime->tm_mon, DateTime->tm_mday,
			DateTime->tm_hour, DateTime->tm_min);
#endif
	}
	else
	{
		count += sprintf(Data + count, "2015-12-16 12:12");
	}

	if (MeasureData)
	{
		//ʹ�ò���ֵ
		count += sprintf(Data + count, "����:    %5.1f��", (float)temp_value / 100);
		count += sprintf(Data + count, "ʪ��:    %3d%%RH", hum_value);
		count += sprintf(Data + count, "����:   %4dW/m2", radg_value);
		count += sprintf(Data + count, "���:   %4dumol", radp_value);
		count += sprintf(Data + count, "����:    %4.1fm/s", (float)ws_value / 100);
		count += sprintf(Data + count, "����:      %3d��", wd_value);
		count += sprintf(Data + count, "00cm����:%5.1f��", (float)temp_value / 100);
		count += sprintf(Data + count, "05cm����:%5.1f��", (float)temp_value / 100);
		count += sprintf(Data + count, "10cm����:%5.1f��", (float)temp_value / 100);
		count += sprintf(Data + count, "����:    %5.1fmm", (float)rf_value / 10);
		count += sprintf(Data + count, "����ˮ��:       ");
		count += sprintf(Data + count, "    10cm:    %2d%%", (sm_value + 5) / 10);
		count += sprintf(Data + count, "    20cm:    %2d%%", (sm_value + 5) / 10);
		count += sprintf(Data + count, "    30cm:    %2d%%", (sm_value + 5) / 10);
	}
	else
	{
		//ʹ�ó�ʼֵ
		count += sprintf(Data + count, "����:    %5.1f��", (float)temp_value / 100);
		count += sprintf(Data + count, "ʪ��:    %3d%%RH", hum_value);
		count += sprintf(Data + count, "����:   %4dW/m2", radg_value);
		count += sprintf(Data + count, "���:   %4dumol", radp_value);
		count += sprintf(Data + count, "����:    %4.1fm/s", (float)ws_value / 100);
		count += sprintf(Data + count, "����:      %3d��", wd_value);
		count += sprintf(Data + count, "00cm����:%5.1f��", (float)temp_value / 100);
		count += sprintf(Data + count, "05cm����:%5.1f��", (float)temp_value / 100);
		count += sprintf(Data + count, "10cm����:%5.1f��", (float)temp_value / 100);
		count += sprintf(Data + count, "����:    %5.1fmm", (float)rf_value / 10);
		count += sprintf(Data + count, "����ˮ��:       ");
		count += sprintf(Data + count, "    10cm:    %2d%%", (sm_value + 5) / 10);
		count += sprintf(Data + count, "    20cm:    %2d%%", (sm_value + 5) / 10);
		count += sprintf(Data + count, "    30cm:    %2d%%", (sm_value + 5) / 10);
	}


	return count;
}






