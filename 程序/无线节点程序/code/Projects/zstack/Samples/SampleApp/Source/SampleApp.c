/**************************************************************************************************
  Filename:       SampleApp.c
  Revised:        $Date: 2009-03-18 15:56:27 -0700 (Wed, 18 Mar 2009) $
  Revision:       $Revision: 19453 $

  Description:    Sample Application (no Profile).


  Copyright 2007 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
  This application isn't intended to do anything useful, it is
  intended to be a simple example of an application's structure.

  This application sends it's messages either as broadcast or
  broadcast filtered group messages.  The other (more normal)
  message addressing is unicast.  Most of the other sample
  applications are written to support the unicast message model.

  Key control:
    SW1:  Sends a flash command to all devices in Group 1.
    SW2:  Adds/Removes (toggles) this device in and out
          of Group 1.  This will enable and disable the
          reception of the flash command.
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "OSAL.h"
#include "ZGlobals.h"
#include "AF.h"
#include "aps_groups.h"
#include "ZDApp.h"

#include "SampleApp.h"
#include "SampleAppHw.h"

#include "OnBoard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"

#include  "MT_UART.h" //此处用于串口
#include  "MT.h"
#include "wdt.h"
#include "adc.h"

#include "APSMEDE.h"
#include "AddrMgr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osal_nv.h"

/*********************************************************************
 * MACROS（宏定义）
 */
#define DATASENSOR_COOR  67
#define DATASENSOR_LEN  27

#define DATA_OFFSET_ID    5
#define DATA_OFFSET_T0    9
#define DATA_OFFSET_U     14
#define DATA_OFFSET_GH    18

#define COOR_DATA_LEN   67

/* AD采样模拟开关通道 */
#define STDR_CHANNEL   0 //标准电阻
#define T0_CHANNEL     1 //温度
#define U_CHANNEL      2 //湿度
#define GH_CHANNEL     3 //光合表

#define AD_VREF         1.25 //AD参考电压

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS(类型定义)
 */
typedef struct
{
  int temperature;
  int humidity;
  int photosyn; //光合量
}dataType_t;

/*********************************************************************
 * GLOBAL VARIABLES(全局变量)
 */
#if (!defined (ZDO_COORDINATOR)) && (!defined (RTR_NWK))  //终端节点

/*入网超时管理*/    /* 16.8.5适当调大离线时间限制，防止只选中单通道时重启时间过短，注意不能超过255 */
#define ORPHAN_MAX   (/*30*/60)   //离线时间限制
uint8 orphan=0;  //孤立节点计数，超过一定时间未连接到网络时重启设备
//uint8 heart=0;   //心跳包计数，超过一定时间未接收到心跳包则重启设备
//uint8 heart_open=0;  //心跳包开关

#endif //终端

int debug = 0; //调试信息开关：1 开，0 关，默认关

int sensor_id = 0; //传感器ID

float gh_factor = 0; //光合表系数

int rs_flag = 0; //复位标志，1 复位，0 无操作

uint8 dev_id[10] = {0}; //保存终端设备ID

uint8 dev_gh[10] = {0}; //保存终端设备光合表系数

uint8 dev_wddt[10] = {0}; //保存终端设备温度订正

float cal_t0 = 0; //温度订正值变量

#if defined (ZDO_COORDINATOR) || defined (RTR_NWK) //协调器
char data_coor[128]={"BG,ID001,-600,099,1234,ID002,-600,099,1234,ID003,-600,099,1234,ED\r\n"}; //协调器数据->主采集器
dataType_t coordi[3]; //协调器数据变量
#else //终端
char data_sensor[128]={"BG,ID001,-600,099,1234,ED\r\n"}; //终端传感器数据->协调器
dataType_t sensor; //传感器数据变量
#endif

// This list should be filled with Application specific Cluster IDs.
const cId_t SampleApp_ClusterList[SAMPLEAPP_MAX_CLUSTERS] =
{
  SAMPLEAPP_PERIODIC_CLUSTERID,
  SAMPLEAPP_FLASH_CLUSTERID
};

const SimpleDescriptionFormat_t SampleApp_SimpleDesc =
{
  SAMPLEAPP_ENDPOINT,              //  int Endpoint;
  SAMPLEAPP_PROFID,                //  uint16 AppProfId[2];
  SAMPLEAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  SAMPLEAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  SAMPLEAPP_FLAGS,                 //  int   AppFlags:4;
  SAMPLEAPP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)SampleApp_ClusterList,  //  uint8 *pAppInClusterList;
  SAMPLEAPP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)SampleApp_ClusterList   //  uint8 *pAppInClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in SampleApp_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t SampleApp_epDesc;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
uint8 SampleApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // SampleApp_Init() is called.
devStates_t SampleApp_NwkState;

uint8 SampleApp_TransID;  // This is the unique message ID (counter)

afAddrType_t SampleApp_Periodic_DstAddr;
afAddrType_t SampleApp_Flash_DstAddr;
afAddrType_t Point_To_Point_DstAddr; //网蜂点对点通信定义

aps_Group_t SampleApp_Group;

uint8 SampleAppPeriodicCounter = 0;
uint8 SampleAppFlashCounter = 0;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void SampleApp_HandleKeys( uint8 shift, uint8 keys );
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void SampleApp_SendPeriodicMessage(char *p, int len );
void SampleApp_SendPointToPointMessage( char *p, int len ); //点对点发送函数
void SampleApp_SerialCMD( mtOSALSerialData_t * MSGpkt); //串口数据处理函数
void SendDataToCollector(char *ptr, int len); //串口向采集器发送数据
void ReturnTorF(int a, int b);//指令回复'T'或'F'
//void Order_Parse(char *str, int len, int flag); //协调器或串口指令解析

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SampleApp_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *          这是一个用户用于初始任务的函数，它是在系统初始化阶段被调用的，而且
 *          在调用它的时候，它里面应该有相应任务的初始化（也就是说，用户有什么
 *          初始化，都是放在这个函数里），比如：硬件初始化、数据表初始化、电源
 *          初始化等等。
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SampleApp_Init( uint8 task_id )
{
  SampleApp_TaskID = task_id;
  SampleApp_NwkState = DEV_INIT;
  SampleApp_TransID = 0;
  
#if defined (ZDO_COORDINATOR) || defined (RTR_NWK) //协调器
  osal_memset(coordi, 0, sizeof(dataType_t)*3); //协调器数据变量初始化
#else //终端
  osal_memset(&sensor, 0, sizeof(dataType_t)); //传感器数据变量初始化
#endif
  
  MT_UartInit(); //串口初始化
  MT_UartRegisterTaskID(task_id); //登记任务号
  printf("System init!\n"); //打印开机初始化信息
  
  ADC_Init(); //AD7705初始化，包括初始化SPI1和复位I/O引脚配置
  
  osal_start_reload_timer( SampleApp_TaskID, SAMPLEAPP_SEND_PERIODIC_MSG_EVT, SAMPLEAPP_SEND_PERIODIC_MSG_TIMEOUT ); //周期5s定时器
  
  osal_start_reload_timer( SampleApp_TaskID, SAMPLEAPP_WDT_PERIODIC_MSG_EVT, SAMPLEAPP_FEEDWDT_PERIODIC_TIMEOUT ); //周期500ms定时器
            
  WDT_Init();//看门狗初始化，放狗
  
#if !(defined (ZDO_COORDINATOR)) || !(defined (RTR_NWK)) //终端
  /* 初始化保存系统参数的flash区域 */
  if(SUCCESS != osal_nv_item_init(DEVICE_ID, DEVICE_ID_LEN, NULL)) //设备ID存入flash区域初始化
    printf("ID flash init failed!!\n");
  
  if(SUCCESS !=osal_nv_item_init(DEVICE_GH, DEVICE_GH_LEN, NULL)) //设备光合表系数存入flash区域初始化
    printf("GH flash init failed!!\n");
  
  if(SUCCESS !=osal_nv_item_init(DEVICE_WDDT, DEVICE_WDDT_LEN, NULL)) //设备温度订正值存入flash区域初始化
    printf("WDDT flash init failed!!\n");
  
  /* 终端传感器读入系统初始化参数，设备ID与光合表系数 */
  if(SUCCESS == osal_nv_read(DEVICE_ID, 0, DEVICE_ID_LEN, dev_id)) //读入flash中存放的设备ID
    sensor_id = atoi(dev_id);
  else
    printf("ID flash read failed!!\n");
  
  if(SUCCESS == osal_nv_read(DEVICE_GH, 0, DEVICE_GH_LEN, dev_gh)) //读入flash中存放的设备光合表系数
    gh_factor = atof(dev_gh);
  else
    printf("GH flash read failed!!\n");
  
  if(SUCCESS == osal_nv_read(DEVICE_WDDT, 0, DEVICE_WDDT_LEN, dev_wddt)) //读入flash中存放的设备温度订正值
    cal_t0 = atof(dev_wddt);
  else
    printf("WDDT flash read failed!!\n");
  
#endif
  
  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().

 #if defined ( BUILD_ALL_DEVICES )
  // The "Demo" target is setup to have BUILD_ALL_DEVICES and HOLD_AUTO_START
  // We are looking at a jumper (defined in SampleAppHw.c) to be jumpered
  // together - if they are - we will start up a coordinator. Otherwise,
  // the device will start as a router.
  if ( readCoordinatorJumper() )
    zgDeviceLogicalType = ZG_DEVICETYPE_COORDINATOR;
  else
    zgDeviceLogicalType = ZG_DEVICETYPE_ROUTER;
#endif // BUILD_ALL_DEVICES

#if defined ( HOLD_AUTO_START )
  // HOLD_AUTO_START is a compile option that will surpress ZDApp
  //  from starting the device and wait for the application to
  //  start the device.
  ZDOInitDevice(0);
#endif

  // Setup for the periodic message's destination address
  // Broadcast to everyone
  SampleApp_Periodic_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
  SampleApp_Periodic_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_Periodic_DstAddr.addr.shortAddr = 0xFFFF;

  // Setup for the flash command's destination address - Group 1
  SampleApp_Flash_DstAddr.addrMode = (afAddrMode_t)afAddrGroup;
  SampleApp_Flash_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_Flash_DstAddr.addr.shortAddr = SAMPLEAPP_FLASH_GROUP;
  
  // 网蜂点对点通讯定义
  Point_To_Point_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;//点播
  Point_To_Point_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  Point_To_Point_DstAddr.addr.shortAddr = 0x0000; //发给协调器

  // Fill out the endpoint description.
  SampleApp_epDesc.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_epDesc.task_id = &SampleApp_TaskID;
  SampleApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&SampleApp_SimpleDesc;
  SampleApp_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &SampleApp_epDesc );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( SampleApp_TaskID );

  // By default, all devices start out in Group 1
  SampleApp_Group.ID = 0x0001;
  osal_memcpy( SampleApp_Group.name, "Group 1", 7  );
  aps_AddGroup( SAMPLEAPP_ENDPOINT, &SampleApp_Group );

#if defined ( LCD_SUPPORTED )
  HalLcdWriteString( "SampleApp", HAL_LCD_LINE_1 );
#endif
}

/*********************************************************************
 * @fn      SampleApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *          事件处理函数（所有的事件都是它管理）
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
uint16 SampleApp_ProcessEvent( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SampleApp_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        case CMD_SERIAL_MSG: //串口收到数据后由 MT_UART 层传递过来的数据，用网蜂方法接收，编译时不定义 MT相关内容 
          SampleApp_SerialCMD((mtOSALSerialData_t *)MSGpkt);
          break;

        // Received when a messages is received (OTA) for this endpoint
        case AF_INCOMING_MSG_CMD: //无线数据接收
          SampleApp_MessageMSGCB( MSGpkt );
          break;

        // Received whenever the device changes state in the network
        case ZDO_STATE_CHANGE:
          SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if ( (SampleApp_NwkState == DEV_ZB_COORD)
              || (SampleApp_NwkState == DEV_ROUTER)
              || (SampleApp_NwkState == DEV_END_DEVICE) ) //网络连接成功
          {
            printf("NET SUCCESS!\r\n");
            switch(SampleApp_NwkState)   //根据_NIB结构中的内容得到设备的短地址、父节点短地址、信道号和网络PanID，并通过串口打印出去
            {
            case DEV_ROUTER:  //路由器节点
              printf("Router:0x%04X\r\nParent:0x%04X\r\nChannel:%d\r\nPanID:0x%04X\r\n",
                      _NIB.nwkDevAddress,_NIB.nwkCoordAddress,_NIB.nwkLogicalChannel,_NIB.nwkPanId);

              printf("IEEE:%02X%02X%02X%02X%02X%02X%02X%02X\r\n",      //IEEE地址
                      aExtendedAddress[7],aExtendedAddress[6],aExtendedAddress[5],aExtendedAddress[4],
                      aExtendedAddress[3],aExtendedAddress[2],aExtendedAddress[1],aExtendedAddress[0]);

              printf("Parent IEEE:%02X%02X%02X%02X%02X%02X%02X%02X\r\n",      //父节点IEEE地址
                      _NIB.nwkCoordExtAddress[7],_NIB.nwkCoordExtAddress[6],_NIB.nwkCoordExtAddress[5],_NIB.nwkCoordExtAddress[4],
                      _NIB.nwkCoordExtAddress[3],_NIB.nwkCoordExtAddress[2],_NIB.nwkCoordExtAddress[1],_NIB.nwkCoordExtAddress[0]);

              break;
            case DEV_END_DEVICE: //终端节点
              printf("EndDevice:0x%04X\r\nParent:0x%04X\r\nChannel:%d\r\nPanID:0x%04X\r\n",
                      _NIB.nwkDevAddress,_NIB.nwkCoordAddress,_NIB.nwkLogicalChannel,_NIB.nwkPanId);

              printf("IEEE:%02X%02X%02X%02X%02X%02X%02X%02X\r\n",      //IEEE地址
                      aExtendedAddress[7],aExtendedAddress[6],aExtendedAddress[5],aExtendedAddress[4],
                      aExtendedAddress[3],aExtendedAddress[2],aExtendedAddress[1],aExtendedAddress[0]);

              printf("Parent IEEE:%02X%02X%02X%02X%02X%02X%02X%02X\r\n",      //父节点IEEE地址
                      _NIB.nwkCoordExtAddress[7],_NIB.nwkCoordExtAddress[6],_NIB.nwkCoordExtAddress[5],_NIB.nwkCoordExtAddress[4],
                      _NIB.nwkCoordExtAddress[3],_NIB.nwkCoordExtAddress[2],_NIB.nwkCoordExtAddress[1],_NIB.nwkCoordExtAddress[0]);

              break;
            case DEV_ZB_COORD:  //协调器
              printf("Coordinator:0x%04X\r\nChannel:%d\r\nPanID:0x%04X\r\n",
                      _NIB.nwkDevAddress,_NIB.nwkLogicalChannel,_NIB.nwkPanId);

              printf("IEEE:%02X%02X%02X%02X%02X%02X%02X%02X\r\n",      //IEEE地址
                      aExtendedAddress[7],aExtendedAddress[6],aExtendedAddress[5],aExtendedAddress[4],
                      aExtendedAddress[3],aExtendedAddress[2],aExtendedAddress[1],aExtendedAddress[0]);

              break;
            default:
              break;
            }
          }
          else
          {
            // Device is no longer in the network
            printf("NO NET\r\n");
            
#if (!defined (ZDO_COORDINATOR)) && (!defined (RTR_NWK))  //终端节点          
            orphan++;  //孤立累计
            if(orphan>ORPHAN_MAX)  //超过指定次数入网失败则重启设备
            {
              rs_flag = 1; //停止喂狗，致使系统复位
            }
#endif //终端
          }
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next - if one is available
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SampleApp_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  // Send a message out - This event is generated by a timer
  //  (setup in SampleApp_Init()).
  /* AD采样事件处理，终端传感器数据采集和发送往协调器 */
  if ( events & SAMPLEAPP_ADC_MEA_EVT )
  {
    int numbers;
    static uint16 res_std;
    uint16 res_t0,res_u,res_gh;
    float temp;
    
    
 #if !(defined (ZDO_COORDINATOR)) && !(defined (RTR_NWK)) //终端
    /* 终端数据采集处理 */
    switch(adc_state)
    {
      case START: //开始测量
            ADC_MeasureStart(STDR_CHANNEL, 0, 3, TRUE); //标准电阻AD转换开始    
            adc_state = READY1; //进入下一状态
            osal_start_timerEx( SampleApp_TaskID, SAMPLEAPP_ADC_MEA_EVT,SAMPLEAPP_ADC_MEA_TIMEOUT ); //200ms定时器事件，用于等待ADC转换完成
            break;
            
      case READY1: //标准电阻AD采样值读取，开启温度ADC转换
            res_std = ADC_ReadData(0);
            ADC_MeasureStart(T0_CHANNEL, 0, 3, TRUE); //温度AD转换开始             
            adc_state = READY2; //进入下一状态
            osal_start_timerEx( SampleApp_TaskID, SAMPLEAPP_ADC_MEA_EVT,SAMPLEAPP_ADC_MEA_TIMEOUT ); //200ms定时器事件，用于等待ADC转换完成
            break;
            
      case READY2: //温度AD采样值读取，开启湿度ADC转换
            res_t0 = ADC_ReadData(0);
            ADC_MeasureStart(U_CHANNEL, 0, 0, TRUE); //湿度AD转换开始
            
            if(res_std==0) //防止除数为零
            {
              res_std = 1;
            }            
            temp = (float)res_t0/(float)res_std*100;
            temp = CalculateTemp(temp) + cal_t0 + CAL_T0_CONSTANT; //加上温度校准值
            
            temp = temp*10; //放大十倍，四舍五入
            if(temp<0)
            {
              temp -= 0.5; 
            }
            else
            {
              temp += 0.5;
            }
             
            sensor.temperature = temp;
            
            if(debug)
            {
              printf("\r\nres_std=%u,res_t0=%u,temper=%5.2f\r\n", res_std, res_t0, temp/10);
            }
            
            adc_state = READY3; //进入下一状态
            osal_start_timerEx( SampleApp_TaskID, SAMPLEAPP_ADC_MEA_EVT,SAMPLEAPP_ADC_MEA_TIMEOUT ); //200ms定时器事件，用于等待ADC转换完成
            break;
            
      case READY3: //湿度AD采样值读取，开启光合量ADC转换
            res_u = ADC_ReadData(0);
            ADC_MeasureStart(GH_CHANNEL, 0, 3, TRUE); //光合量AD转换开始 ，增益8倍          
            sensor.humidity = (float)res_u/65535*AD_VREF*100 + 0.5;
            
            if(debug)
            {
              printf("\r\nres_u=%u,humidity=%d\r\n", res_u, sensor.humidity);
            }
            
            adc_state = READY4; //进入下一状态
            osal_start_timerEx( SampleApp_TaskID, SAMPLEAPP_ADC_MEA_EVT,SAMPLEAPP_ADC_MEA_TIMEOUT ); //200ms定时器事件，用于等待ADC转换完成
            break;
            
      case READY4: //光合量AD采样值读取，转换完成
            res_gh = ADC_ReadData(0);
            temp = (float)res_gh/65535*AD_VREF/8*1000; //乘1000电压单位转换为mV
            sensor.photosyn = CalculateRadiation(temp, gh_factor) + 0.5 + CAL_GH; //加上光合整体订正值
            if(debug)
            {
              printf("\r\nres_gh=%u,guanghe=%d\r\n", res_gh, sensor.photosyn);
            }
            adc_done = TRUE;
            break;
    }
    
    if(adc_done) //测量转换完成，数据处理发送
    {
          
      adc_state = START; //初始化
      
      adc_done = FALSE; //清零，为下次转换做准备
      
      numbers = sprintf(data_sensor, "BG,ID%03u,%04d,%03d,%04d,ED\r\n", sensor_id, 
                        sensor.temperature, sensor.humidity, sensor.photosyn);
      
      /* 终端数据发送 */
      if(debug)
      {
        printf("\r\n%s", data_sensor);
      }
      
      SampleApp_SendPointToPointMessage(data_sensor, numbers); //点对点发送函数,终端传感器数据发往协调器
    }
#endif

    // return unprocessed events
    return (events ^ SAMPLEAPP_ADC_MEA_EVT);
  }
  
  /* 周期 5s 事件处理 */
  if( events & SAMPLEAPP_SEND_PERIODIC_MSG_EVT )
  {
#if !(defined (ZDO_COORDINATOR)) && !(defined (RTR_NWK)) //终端
    osal_set_event( SampleApp_TaskID, SAMPLEAPP_ADC_MEA_EVT );//触发ADC采样处理事件
#endif
    // return unprocessed events
    return (events ^ SAMPLEAPP_SEND_PERIODIC_MSG_EVT);
  }
  
  /* 周期500ms喂狗事件处理 */
  if( events & SAMPLEAPP_WDT_PERIODIC_MSG_EVT )
  {
    if(!rs_flag)
    {
      WDT_Feed();//喂狗
      //HalUARTWrite(0,"hellow!\n",strlen("hellow!\n") );
    }
  }

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * Event Generation Functions
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      SampleApp_MessageMSGCB
 *
 * @brief   这个函数管理所有接受到的数据，至于数据来自哪个设备，它是根据
 *          簇ID来分辨的。
 *          所有接收到的数据都会存入 afIncomingMSGPacket_t *pkt所指向的存储区域
 *          内
 *          
 *          
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pkt ) //2017.03.17 杨广湘：去除簇ID方式解析信息，
{
  char *str = &pkt->cmd.Data[0];
  int len = pkt->cmd.DataLength;
  int sens_id = 0;
  char temp[20];
  int numbers;
  
  /* 无线接收数据打印 */
  if(debug) //调试信息
  {
    HalUARTWrite(0, str,len);//打印收到数据
  }
  
  /* 无线接收数据解析 */
  if(*(str+len-1)== '\n')
  {
#if defined (ZDO_COORDINATOR) || defined (RTR_NWK) //协调器
    /* 协调器接收数据解析 */
    sens_id = atoi(str+DATA_OFFSET_ID);
    if(sens_id>=1&&sens_id<=3) //提取传感器数据
    {
      int i = 0;
      i = sens_id - 1;
      coordi[i].temperature = atoi(str+DATA_OFFSET_T0);
      coordi[i].humidity = atoi(str+DATA_OFFSET_U);
      coordi[i].photosyn = atoi(str+DATA_OFFSET_GH);
    }
    
#else //终端
    /* 终端接收数据解析 */
    if(osal_memcmp(str,"#RS",3))//复位
    {
      if((len>5)&&(atoi(str+4)!=0))  //参数不为0清除掉网络信息
      {
        zgWriteStartupOptions(ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_NETWORK_STATE);
        printf("clear\r\n");
      }
      
      printf("reset\r\n");
      /* 复位 */     
      rs_flag = 1;//置位停止喂狗标志     
    }
    else if(osal_memcmp(str,"#GH",3))
    {
      if(len == 5) //读取光合表系数
      {
        numbers = sprintf(temp,"<GH,%03u,%.2f>\r\n", sensor_id, gh_factor);
        SampleApp_SendPointToPointMessage(temp, numbers);
      }
      else //设置光合表系数
      {
        float temp_gh;
        int temp_id;
        char *p = NULL;
        
        p = strchr(str, ','); //查找第一个","出现的位置
        sscanf(p+1, "%d", &temp_id);
        if(temp_id == sensor_id)
        {
          p = strchr(p+1, ','); //查找第二个","出现的位置
          sscanf(p+1, "%f", &temp_gh);
          if(temp_gh>=1.0)
          {
            gh_factor = temp_gh;
            
            /* 光合表系数存入flash */
            sprintf(dev_gh, "%5.2f", gh_factor);
            
            if(SUCCESS == osal_nv_write(DEVICE_GH, 0, DEVICE_GH_LEN, dev_gh))
              ReturnTorF(1,0); //存入成功
            else
              ReturnTorF(0,0); //存入失败
          }
          else
          {
            ReturnTorF(0,0); //错误
          }
        }
      }
    }
#endif
  }

}

/*********************************************************************
 * @fn      SampleApp_SendPeriodicMessage
 *
 * @brief   Send the periodic message.
 *          这个函数，是一个周期性数据发送函数，它是设备设置为周期性广播的时候
 *          调用的，它是我们设置发送数据的地方
 *   SAMPLEAPP_PERIODIC_CLUSTERID,  这是一个簇，定义的作用是和接收方建立联系，
 *           协调器收到这个标号，如果是1，就是证明是由周期性广播方式发送过来的
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_SendPeriodicMessage(char *p, int len)
{
  if ( AF_DataRequest( &SampleApp_Periodic_DstAddr, &SampleApp_epDesc,
                       SAMPLEAPP_PERIODIC_CLUSTERID,  
                       len,//字节数
                       p,//指针头
                       &SampleApp_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
  }
  else
  {
    // Error occurred in request to send.
  }
}

/*********************************************************************
 * @fn      SampleApp_SendPointToPointMessage
 *
 * @brief   网蜂点对点发送函数
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_SendPointToPointMessage( char *p, int len )
{ 
  if ( AF_DataRequest( &Point_To_Point_DstAddr,&SampleApp_epDesc,
                         SAMPLEAPP_POINT_TO_POINT_CLUSTERID,
                         len,//字节数
                         p,//首地址  要发送数据的首地址
                         &SampleApp_TransID,
                         AF_DISCV_ROUTE,
                         AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
  }
  else
  {
    // Error occurred in request to send.
  }
}

/*********************************************************************
 * @fn      SampleApp_SerialCMD
 *
 * @brief   通讯串口接收数据解析
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_SerialCMD(mtOSALSerialData_t *cmdMsg)
{
   uint8 i,len,*str=NULL; //len 有用数据长度
   int numbers;
   str=cmdMsg->msg; //指向数据开头
   len=*str; //msg 里的第 1 个字节代表后面的数据长度
   str++; //令指针指向第一个字符
   
   /********打印出串口接收到的数据，用于提示*********/
   //HalUARTWrite(0,str,len);
   
   /* 上位机指令解析 */
   if(*(str+len-1)== '\n')
   {
#if defined (ZDO_COORDINATOR) || defined (RTR_NWK) //协调器
     
      //数据读取（DM）
      if(osal_memcmp(str,"#DM",3)) //主采读取数据
      {
        /* 主采读取数据 */
        numbers = sprintf(data_coor, "BG,ID001,%04d,%03d,%04d,ID002,%04d,%03d,%04d,ID003,%04d,%03d,%04d,ED\r\n",coordi[0].temperature,
                          coordi[0].humidity,coordi[0].photosyn,coordi[1].temperature,coordi[1].humidity,coordi[1].photosyn,
                          coordi[2].temperature,coordi[2].humidity,coordi[2].photosyn); //数据填充
        
        SendDataToCollector(data_coor, numbers); //数据发往主采
        
        osal_memset(coordi, 0, sizeof(dataType_t)*3); //发送后数据清零
      }
      
      //复位指令（RS）
      else if(osal_memcmp(str,"#RS",3))
      {
        /* 复位 */
        if((len>5)&&(atoi(str+4)!=0))  //参数不为0清除掉网络信息
        {
          zgWriteStartupOptions(ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_NETWORK_STATE);
          printf("clear\r\n");
        }
        
        ReturnTorF( 1, 1 ); //正确
        SampleApp_SendPeriodicMessage(str, len); //转发复位指令给终端传感器
        
        rs_flag = 1;//置位停止喂狗标志
        
      }
      else if(osal_memcmp(str,"#GH",3)) //协调器使用GH指令给传感器设置光合表系数
      {
        SampleApp_SendPeriodicMessage(str, len);
      }
#else //终端节点
      
      //设备ID读取和设置（ID）
      if(osal_memcmp(str,"#ID",3))
      {
        if(len == 5) //读取设备ID
        {          
          printf("<ID,%03u>\r\n", sensor_id);
        }
        else //设置设备ID
        {
          int temp_id;
          
          sscanf(str+4, "%d", &temp_id);                           
          if(temp_id>=1 && temp_id<=3)
          {
            sensor_id = temp_id;
            
            /* 设备ID存入flash */
            sprintf(dev_id, "%03u", sensor_id);
            
            if(SUCCESS == osal_nv_write(DEVICE_ID, 0, DEVICE_ID_LEN, dev_id))
              ReturnTorF( 1, 1 ); //存入成功
            else
              ReturnTorF( 0, 1 ); //存入失败
          }
          else
          {
            ReturnTorF( 0, 1 ); //错误
          }
        }
      }
      
      //光合表读取和设置（GH）
      else if(osal_memcmp(str,"#GH",3))
      {
        if(len == 5) //读取光合表系数
        {
          printf("<GH,%03u,%.2f>\r\n", sensor_id, gh_factor);
        }
        else //设置光合表系数
        {
          float temp_gh;
          char *p = NULL;
          
          p = strchr(str, ','); //查找第一个","出现的位置
          p = strchr(p+1, ','); //查找第二个","出现的位置
          sscanf(p+1, "%f", &temp_gh);
          if(temp_gh>=1.0)
          {
            gh_factor = temp_gh;
            
            /* 光合表系数存入flash */
            sprintf(dev_gh, "%5.2f", gh_factor);
            
            if(SUCCESS == osal_nv_write(DEVICE_GH, 0, DEVICE_GH_LEN, dev_gh))
              ReturnTorF( 1, 1 ); //存入成功
            else
              ReturnTorF( 0, 1 ); //存入失败
          }
          else
          {
            ReturnTorF( 0, 1 ); //错误
          }
        }
      }
      
      //复位指令（RS）
      else if(osal_memcmp(str,"#RS",3)) 
      {
        /* 复位 */
        if((len>5)&&(atoi(str+4)!=0))  //参数不为0清除掉网络信息
        {
          zgWriteStartupOptions(ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_NETWORK_STATE);
          printf("clear\r\n");
        }
        ReturnTorF( 1, 1 );//复位成功
        rs_flag = 1;//置位停止喂狗标志
      }
      
      //温度订正（CR）
      else if(osal_memcmp(str,"#CR",3)) //温度订正
      {
        if( len == 5 ) //读取温度订正值
        {
          printf("<CR,%.2f>\r\n", cal_t0);
        }
        else if( len > 5 )
        {
          float temp_wddt;
          char *p = NULL;
            
          p = strchr(str, ','); //查找第一个","出现的位置
          sscanf( p+1, "%f", &temp_wddt);
          if( temp_wddt>-10.0 && temp_wddt<10.00 )
          {
            cal_t0 = temp_wddt;
             /* 温度订正存入flash */
            sprintf(dev_wddt, "%5.2f", cal_t0);
            
            if(SUCCESS == osal_nv_write(DEVICE_WDDT, 0, DEVICE_WDDT_LEN, dev_wddt))
              ReturnTorF( 1, 1 ); //存入成功
            else
              ReturnTorF( 0, 1 ); //存入失败
            
          }
          else
          {
            ReturnTorF( 0, 1 );
          }
           
        }
        else
        {
          ReturnTorF( 0, 1 );
        }
      }
#endif
      //查询或设置PANID（PD）
      else if(osal_memcmp(str,"#PD",3))
      {
        if(len==5)  //读取
        {
          printf("<PD,0x%04X>\r\n", _NIB.nwkPanId);
        }
        else if(len>5)  //设置
        {
          uint16 panid=0;
          
          sscanf(str+4,"%x",&panid);  //得到新的PanID
          _NIB.nwkPanId=panid;  //更新PanID
          
          /* 2017.4.10添加使能NV_INIT和NV_RESTORE时设置channel和panid时同时清除掉节点保存的协调器网络信息，
             这样便可以在更改channel或panid后加入有相同channel和panid的任意协调器的网络而不会更换协调器后无法入网，
             但还是需要重启节点才能加入新网络，而且PANID编译时需要设为0xFFFF，channel要选则全部11-26通道 */
          _NIB.nwkCoordAddress = INVALID_NODE_ADDR;
          osal_memset( _NIB.nwkCoordExtAddress, 0, Z_EXTADDR_LEN );
          /* */
          
          NLME_UpdateNV(0x01);  //将 NIB 信息存入 NV 中，重启后使用新的PanID
          
          ReturnTorF( 1, 1 );
        }
        else
        {
          ReturnTorF( 0, 1 );
        }
      }
      
      //查询设置信道号（CH）
      else if(osal_memcmp(str,"#CH",3))
      {
        if(len==5)  //读取
        {
          printf("<CH,0x%02X>\r\n", _NIB.nwkLogicalChannel);
        }
        else if(len>5)  //设置
        {
          uint16 channel=0;
          
          sscanf(str+4,"%x",&channel);  //得到新的channel
          _NIB.nwkLogicalChannel=channel;  //更新channel
          
          /* 2017.4.10添加使能NV_INIT和NV_RESTORE时设置channel和panid时同时清除掉节点保存的协调器网络信息，
             这样便可以在更改channel或panid后加入有相同channel和panid的任意协调器的网络而不会更换协调器后无法入网，
             但还是需要重启节点才能加入新网络，而且PANID编译时需要设为0xFFFF，channel要选则全部11-26通道 */
          _NIB.nwkCoordAddress = INVALID_NODE_ADDR;
          osal_memset( _NIB.nwkCoordExtAddress, 0, Z_EXTADDR_LEN );
          /* */
          
          NLME_UpdateNV(0x01);  //将 NIB 信息存入 NV 中，重启后应用
          
          ReturnTorF( 1, 1 );
        }
        else
        {
          ReturnTorF( 0, 1 );
        }
      }
      
      //查询短地址（SA）
      else if(osal_memcmp(str,"#SA",3))
      {
        printf("<SA,0x%04X>\r\n", _NIB.nwkDevAddress);
      }
      
      /* 2017.4.10添加修改入网时默认的chennel和panid，不打开NV_INIT和NV_RESTORE；
         仅终端节点添加，协调器还是使用NV_INIT和NV_RESTORE保持不变，因为协调器需要保存网络信息如邻居表等，
         这样终端节点重新入网后地址等信息不会改变 */
      /* 16.8.5添加DCH和CPD命令读取和设置默认入网信道和PANID，这样原来的CH和PD命令用来查看当前网络的信道和PANID */
      //查询或设置默认入网PANID（DPD）
      else if(osal_memcmp(str,"#DPD",3))
      {
        if(len==6)  //读取
        {
          printf("<DPD,0x%04X>\r\n", zgConfigPANID);   /* 默认PANID */
        }
        else if(len>6)  //设置
        {
          uint16 panid=0;
          
          sscanf(str+5,"%x",&panid);  //得到新的PanID
          
          extern uint8 nvItemInit( uint16 id, uint16 len, void *buf, uint8 set );
          zgConfigPANID = panid;   /* 修改默认panid */
          //保存到NV
          if(nvItemInit(ZCD_NV_PANID,sizeof(zgConfigPANID), &zgConfigPANID,TRUE) != ZSUCCESS)
          {
            printf("<DPD,F>\r\n");
          }
          else
          {
            printf("<DPD,T>\r\n");
          }
          
        }
        else
        {
          printf("<DPD,F>\r\n");
        }
      }
      
      
      //查询设置默认入网信道号（DCH）
      else if(osal_memcmp(str,"#DCH",4))
      {
        if(len==6)  //读取
        {
          if(zgDefaultChannelList == MAX_CHANNELS_24GHZ)  //通道全选
          {
            printf("<DCH,ALL>\r\n");
          }
          else
          {
            for(i=11;i<=26;i++)  //根据channel list得到信道，信道设置时只选一个或者全选
            {
              if(zgDefaultChannelList&((uint32)1<<i)) break;
            }
            printf("<DCH,0x%02X>\r\n", i);
          }
        }
        else if(len>6)  //设置
        {
          uint16 channel=0;
          
          sscanf(str+5,"%x",&channel);  //得到新的channel
          
          /* 2017.4.10如果信道设置为"ALL"或"all"时表示通道全选，此时将channel设为0表示通道全选 */
          if(osal_memcmp(str+5,"ALL",3) || osal_memcmp(str+5,"all",3))  //11-26通道全选
          {
            channel=0;
          }
          
          extern uint8 nvItemInit( uint16 id, uint16 len, void *buf, uint8 set );
          if(((channel<11) || (channel>26)) && (channel!=0))  //无效信道且不为0
          {
            printf("<CH,F>\r\n");
          }
          else
          {
            //根据channel得到channel list，注意强制类型转换
            if(channel==0)  //为0时将信道全选
            {
              zgDefaultChannelList=MAX_CHANNELS_24GHZ;
            }
            else
            {
              zgDefaultChannelList=(uint32)((uint32)1<<channel);
            }
            printf("<CH,0x%02X,0x%08lX>\r\n", channel, zgDefaultChannelList);
            //保存到NV
            if(nvItemInit(ZCD_NV_CHANLIST,sizeof(zgDefaultChannelList), &zgDefaultChannelList,TRUE) != ZSUCCESS)
            {
              printf("<DCH,F>\r\n");
            }
            else
            {
              printf("<DCH,T>\r\n");
            }
          }
        }
        else
        {
          printf("<DCH,F>\r\n");
        }
      }
      
      //调试信息开关（DG）
      else if(osal_memcmp(str,"#DG",3)) 
      {
        int temp;
        
        temp = atoi(str+4);
        if(temp==0||temp==1)
        {
          debug = temp;
          ReturnTorF( 1, 1 ); //正确
        }
        else
        {
          ReturnTorF( 0, 1 ); //错误
        }
        
      }
      else
      {
        ReturnTorF( 0, 1 ); //错误
      }
   }
   else
   {
      ReturnTorF( 0, 1 ); //错误
   }
   
}

/*********************************************************************
 * @fn      sendData
 *
 * @brief   串口向采集器发送数据
 *
 * @param   none
 *
 * @return  none
 */
void SendDataToCollector(char *ptr, int len)
{
    HalUARTWrite(0,ptr,len);
}

/*********************************************************************
 * @fn      Order_Parse
 *
 * @brief   协调器或串口指令解析,flag：1 指示终端设备，0 指示协调器设备
 *
 * @param   none
 *
 * @return  none
 */
//void Order_Parse(char *str, int len, int flag)
//{
//   uint8 i;
//   int numbers;
//     /* 上位机指令解析 */
//   if(*(str+len-1)== '\n')
//   {
//#if defined (ZDO_COORDINATOR) || defined (RTR_NWK) //协调器
//      if(osal_memcmp(str,"#DM",3)) //主采读取数据
//      {
//        /* 主采读取数据 */
//        numbers = sprintf(data_coor, "BG,ID001,%04d,%03d,%04d,ID002,%04d,%03d,%04d,ID003,%04d,%03d,%04d,ED\r\n",coordi[0].temperature,
//                          coordi[0].humidity,coordi[0].photosyn,coordi[1].temperature,coordi[1].humidity,coordi[1].photosyn,
//                          coordi[2].temperature,coordi[2].humidity,coordi[2].photosyn); //数据填充
//        
//        SendDataToCollector(data_coor, COOR_DATA_LEN); //数据发往主采集器
//        
//        osal_memset(coordi, 0, sizeof(dataType_t)*3); //发送后数据清零
//      }
//      else if(osal_memcmp(str,"#RS",3))
//      {
//        /* 复位 */
//        ReturnTorF(1, 1); //正确
//        SampleApp_SendPeriodicMessage(str, len); //转发复位指令给终端传感器
//        
//        rs_flag = 1;//置位停止喂狗标志
//        
//      }
//      else if(osal_memcmp(str,"#GH",3)) //协调器使用GH指令给传感器设置光合表系数
//      {
//        SampleApp_SendPeriodicMessage(str, len);
//      }
//#else //终端节点
//      if(osal_memcmp(str,"#ID",3))
//      {
//        if(len == 5) //读取设备ID
//        {          
//          printf("<ID,%03u>\r\n", sensor_id);
//        }
//        else //设置设备ID
//        {
//          int temp_id;
//          
//          sscanf(str+4, "%d", &temp_id);                           
//          if(temp_id>=1 && temp_id<=3)
//          {
//            sensor_id = temp_id;
//            
//            /* 设备ID存入flash */
//            sprintf(dev_id, "%03u", sensor_id);
//            
//            if(SUCCESS == osal_nv_write(DEVICE_ID, 0, DEVICE_ID_LEN, dev_id))
//              HalUARTWrite(0,"T\n",2 ); //存入成功
//            else
//              HalUARTWrite(0,"F\n",2 ); //存入失败
//          }
//          else
//          {
//            HalUARTWrite(0,"F\n",2 ); //错误
//          }
//        }
//      }
//      else if(osal_memcmp(str,"#GH",3))
//      {
//        if(len == 5) //读取光合表系数
//        {
//          printf("<GH,%05.2f>\r\n", gh_factor);
//        }
//        else //设置光合表系数
//        {
//          float temp_gh;
//          
//          sscanf(str+4, "%f", &temp_gh);
//          if(temp_gh>=1.0)
//          {
//            gh_factor = temp_gh;
//            
//            /* 光合表系数存入flash */
//            sprintf(dev_gh, "%5.2f", gh_factor);
//            
//            if(SUCCESS == osal_nv_write(DEVICE_GH, 0, DEVICE_GH_LEN, dev_gh))
//              HalUARTWrite(0,"T\n",2 ); //存入成功
//            else
//              HalUARTWrite(0,"F\n",2 ); //存入失败
//          }
//          else
//          {
//            HalUARTWrite(0,"F\n",2 ); //错误
//          }
//        }
//      }
//      else if(osal_memcmp(str,"#RS",3))//复位
//      {
//        /* 复位 */     
//        rs_flag = 1;//置位停止喂狗标志     
//      }
//#endif
//      else if(osal_memcmp(str,"#DG",3)) //调试信息开关
//      {
//        int temp;
//        
//        temp = atoi(str+4);
//        if(temp==0||temp==1)
//        {
//          debug = temp;
//          HalUARTWrite(0,"T\n",2 ); //正确
//        }
//        else
//        {
//          HalUARTWrite(0,"F\n",2 ); //错误
//        }
//        
//      }
//      else
//      {
//        HalUARTWrite(0,"F\n",2 ); //错误
//      }
//   }
//   else
//   {
//      HalUARTWrite(0,"F\n",2 ); //错误
//   }
//}

/*********************************************************************
 * @fn      ReturnTorF
 *
 * @brief   指令返回'T'或'F'，参数a:1 回复'T',0 回复'F'；参数b:1 发往串口，0 发往协调器
 *
 * @param   none
 *
 * @return  none
 */
void ReturnTorF(int a, int b)
{
  if(a)
  {
    if(b)
    {
      printf("T\r\n");
    }
    else
    {
      SampleApp_SendPointToPointMessage("T\r\n", sizeof("T\r\n"));
    }
  }
  else
  {
    if(b)
    {
      printf("F\r\n");
    }
    else
    {
      SampleApp_SendPointToPointMessage("F\r\n", sizeof("F\r\n"));
    }
    
  }
  
}

/*********************************************************************
*********************************************************************/
