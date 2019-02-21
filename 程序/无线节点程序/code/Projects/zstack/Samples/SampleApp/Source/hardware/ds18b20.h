#ifndef __DS18B20_H__
#define __DS18B20_H__


#define Ds18b20Data P0_7 //温度传感器引脚

#define ON 0x01  //读取成功返回0x00，失败返回0x01
#define OFF 0x00

extern uint8 Ds18b20Initial(void);
extern int16 Temp_Get(void);

#endif

