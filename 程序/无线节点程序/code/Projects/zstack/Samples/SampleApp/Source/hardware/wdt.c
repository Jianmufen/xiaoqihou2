#include "wdt.h"

void WDT_Init(void)
{
  WDCTL=0x00;
  WDCTL|=0x08;   //设置为看门狗模式,间隔1s
}

void WDT_Feed(void)   //喂狗
{
  WDCTL=0xA0;
  WDCTL=0x50;
}
