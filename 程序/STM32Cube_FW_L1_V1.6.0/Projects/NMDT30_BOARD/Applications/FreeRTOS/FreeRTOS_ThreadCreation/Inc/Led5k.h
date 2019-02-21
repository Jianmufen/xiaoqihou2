#ifndef __LED5K_H
#define __LED5K_H

#include "led5k_def.h"
#include "time.h"


#ifdef __cplusplus
extern "C" {
#endif 


//�������ͼ������
uint32_t Led5kFillAreaText(uint8_t *Data, const struct tm *DateTime, const void *MeasureData, const uint8_t *InputText, uint32_t InputLength);
//����������ݣ���������ͷ+����ͼ������
uint32_t Led5kFillAreaData(uint8_t *Data, const struct tm *DateTime, const void *MeasureData, const uint8_t *InputText, uint32_t InputLength);
//��䶯̬�������ݣ���̬����ͷ+�������ݣ�ֻʹ��һ������0��
uint32_t Led5kFillDynamicData(uint8_t *Data, const struct tm *DateTime, const void *MeasureData, const uint8_t *InputText, uint32_t InputLength);
//���PHY1�㣺��ͷ����(14byte) | ������(Nbyte) | ��У��(2byte) 
uint32_t Led5kFillPHY1Data(uint8_t *Data, const struct tm *DateTime, const void *MeasureData, const uint8_t *InputText, uint32_t InputLength);
//��������֡
int32_t SendDynamicArea(uint8_t *Data, uint32_t Length);
//��������
void Led5kSetScreenParam(void);


#ifdef __cplusplus
}
#endif

#endif /* __LED5K_H */