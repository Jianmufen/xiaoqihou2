#ifndef __LED5K_CONF_H
#define __LED5K_CONF_H


#ifdef __cplusplus
extern "C" {
#endif 


/* LED��Ļ���壬��ͬ�ط��ߴ�����ݲ�ͬ */
typedef enum
{
	LED_DefaultLocation=0,       /* Ĭ�ϣ�128*80 */
	LED_QingDao_JiaoZhou = 1,    /* �ൺ���ݣ�128*80 */
} LED_LocationTypeDef;

#define LED_LOCATION   LED_QingDao_JiaoZhou



#if (LED_LOCATION==LED_QingDao_JiaoZhou)

/* LED��Ļ��̬����ߴ� */
#define LED_SCREEN_WIDTH   (128)
#define LED_SCREEN_HEIGHT  (80)

/* ������Ļ�������� */
#define LED_SET_SCREEN_PARAM  (ScreenParam_128_80)

#else

/* LED��Ļ��̬����ߴ� */
#define LED_SCREEN_WIDTH   (128)
#define LED_SCREEN_HEIGHT  (80)

/* ������Ļ�������� */
#define LED_SET_SCREEN_PARAM  (ScreenParam_128_80)

#endif   /* LED_LOCATION */




#ifdef __cplusplus
}
#endif

#endif /* __LED5K_H */