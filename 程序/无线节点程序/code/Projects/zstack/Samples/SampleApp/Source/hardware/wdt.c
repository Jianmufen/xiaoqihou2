#include "wdt.h"

void WDT_Init(void)
{
  WDCTL=0x00;
  WDCTL|=0x08;   //����Ϊ���Ź�ģʽ,���1s
}

void WDT_Feed(void)   //ι��
{
  WDCTL=0xA0;
  WDCTL=0x50;
}
