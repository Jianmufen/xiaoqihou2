#ifndef __SPI_H
#define __SPI_H

#include "ioCC2530.h"
#include "OSAL.h"
#include "hal_board.h"
#include "OnBoard.h"
#include "SampleApp.h"
#include "hal_defs.h"
#include "wdt.h"

#define SSN P1_4  //SPIÆ¬Ñ¡Òý½Å


void SPI1_Init(uint8 BAUD_E,uint8 BAUD_M);
void SPI1_SetSpeed(uint8 BAUD_E,uint8 BAUD_M);
uint8 SPI1_ReadWriteByte(uint8 TxData);



#endif
