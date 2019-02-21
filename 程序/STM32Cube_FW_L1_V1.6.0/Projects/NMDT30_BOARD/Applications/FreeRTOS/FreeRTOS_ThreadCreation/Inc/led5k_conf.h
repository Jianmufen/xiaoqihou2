#ifndef __LED5K_CONF_H
#define __LED5K_CONF_H


#ifdef __cplusplus
extern "C" {
#endif 


/* LED��Ļ���壬��ͬ�ط��ߴ�����ݲ�ͬ */  /* 16.6.14���ú궨�壬��ʹ��ö�٣�ö�ٲ�����ȷ���� */
//typedef enum
//{
//	LED_DefaultLocation=0,       /* Ĭ�ϣ�128*80 */
//	LED_QingDao_JiaoZhou = 1,    /* �ൺ���ݣ�128*80 */
//    LED_BeiJing_ZhanLan = 2,     /* 16.6.14����չ����96*32 */
//} LED_LocationTypeDef;
#define	LED_DefaultLocation     0       /* Ĭ�ϣ�128*80 */
#define	LED_QingDao_JiaoZhou    1       /* �ൺ���ݣ�128*80 */
#define LED_BeiJing_ZhanLan     2       /* 16.6.14����չ����96*32 */


/* ������ѡ��LED��Ļ��װ�ص㣬��ͬ�ص�ߴ粻ͬ */
#define LED_LOCATION   LED_BeiJing_ZhanLan

/* ����������ÿһ��ͣ��ʱ�䣬��λΪs */
#define LED_STAY_TIME  (3)



#if (LED_LOCATION==LED_QingDao_JiaoZhou)                         /* �ൺ���� */

/* LED��Ļ��̬����ߴ� */
#define LED_SCREEN_WIDTH   (128)
#define LED_SCREEN_HEIGHT  (80)

/* ������Ļ�������� */
#define LED_SET_SCREEN_PARAM  (ScreenParam_128_80)




#elif (LED_LOCATION==LED_BeiJing_ZhanLan)                        /* ����չ�� */

/* LED��Ļ��̬����ߴ� */
#define LED_SCREEN_WIDTH   (96)
#define LED_SCREEN_HEIGHT  (32)

/* ������Ļ�������� */
#define LED_SET_SCREEN_PARAM  (ScreenParam_96_32)




#else                                                            /* Ĭ�� */

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