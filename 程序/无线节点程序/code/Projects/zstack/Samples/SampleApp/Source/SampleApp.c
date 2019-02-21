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
  PROVIDED �AS IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

#include  "MT_UART.h" //�˴����ڴ���
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
 * MACROS���궨�壩
 */
#define DATASENSOR_COOR  67
#define DATASENSOR_LEN  27

#define DATA_OFFSET_ID    5
#define DATA_OFFSET_T0    9
#define DATA_OFFSET_U     14
#define DATA_OFFSET_GH    18

#define COOR_DATA_LEN   67

/* AD����ģ�⿪��ͨ�� */
#define STDR_CHANNEL   0 //��׼����
#define T0_CHANNEL     1 //�¶�
#define U_CHANNEL      2 //ʪ��
#define GH_CHANNEL     3 //��ϱ�

#define AD_VREF         1.25 //AD�ο���ѹ

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS(���Ͷ���)
 */
typedef struct
{
  int temperature;
  int humidity;
  int photosyn; //�����
}dataType_t;

/*********************************************************************
 * GLOBAL VARIABLES(ȫ�ֱ���)
 */
#if (!defined (ZDO_COORDINATOR)) && (!defined (RTR_NWK))  //�ն˽ڵ�

/*������ʱ����*/    /* 16.8.5�ʵ���������ʱ�����ƣ���ֹֻѡ�е�ͨ��ʱ����ʱ����̣�ע�ⲻ�ܳ���255 */
#define ORPHAN_MAX   (/*30*/60)   //����ʱ������
uint8 orphan=0;  //�����ڵ����������һ��ʱ��δ���ӵ�����ʱ�����豸
//uint8 heart=0;   //����������������һ��ʱ��δ���յ��������������豸
//uint8 heart_open=0;  //����������

#endif //�ն�

int debug = 0; //������Ϣ���أ�1 ����0 �أ�Ĭ�Ϲ�

int sensor_id = 0; //������ID

float gh_factor = 0; //��ϱ�ϵ��

int rs_flag = 0; //��λ��־��1 ��λ��0 �޲���

uint8 dev_id[10] = {0}; //�����ն��豸ID

uint8 dev_gh[10] = {0}; //�����ն��豸��ϱ�ϵ��

uint8 dev_wddt[10] = {0}; //�����ն��豸�¶ȶ���

float cal_t0 = 0; //�¶ȶ���ֵ����

#if defined (ZDO_COORDINATOR) || defined (RTR_NWK) //Э����
char data_coor[128]={"BG,ID001,-600,099,1234,ID002,-600,099,1234,ID003,-600,099,1234,ED\r\n"}; //Э��������->���ɼ���
dataType_t coordi[3]; //Э�������ݱ���
#else //�ն�
char data_sensor[128]={"BG,ID001,-600,099,1234,ED\r\n"}; //�ն˴���������->Э����
dataType_t sensor; //���������ݱ���
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
afAddrType_t Point_To_Point_DstAddr; //�����Ե�ͨ�Ŷ���

aps_Group_t SampleApp_Group;

uint8 SampleAppPeriodicCounter = 0;
uint8 SampleAppFlashCounter = 0;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void SampleApp_HandleKeys( uint8 shift, uint8 keys );
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void SampleApp_SendPeriodicMessage(char *p, int len );
void SampleApp_SendPointToPointMessage( char *p, int len ); //��Ե㷢�ͺ���
void SampleApp_SerialCMD( mtOSALSerialData_t * MSGpkt); //�������ݴ�����
void SendDataToCollector(char *ptr, int len); //������ɼ�����������
void ReturnTorF(int a, int b);//ָ��ظ�'T'��'F'
//void Order_Parse(char *str, int len, int flag); //Э�����򴮿�ָ�����

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
 *          ����һ���û����ڳ�ʼ����ĺ�����������ϵͳ��ʼ���׶α����õģ�����
 *          �ڵ�������ʱ��������Ӧ������Ӧ����ĳ�ʼ����Ҳ����˵���û���ʲô
 *          ��ʼ�������Ƿ����������������磺Ӳ����ʼ�������ݱ��ʼ������Դ
 *          ��ʼ���ȵȡ�
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
  
#if defined (ZDO_COORDINATOR) || defined (RTR_NWK) //Э����
  osal_memset(coordi, 0, sizeof(dataType_t)*3); //Э�������ݱ�����ʼ��
#else //�ն�
  osal_memset(&sensor, 0, sizeof(dataType_t)); //���������ݱ�����ʼ��
#endif
  
  MT_UartInit(); //���ڳ�ʼ��
  MT_UartRegisterTaskID(task_id); //�Ǽ������
  printf("System init!\n"); //��ӡ������ʼ����Ϣ
  
  ADC_Init(); //AD7705��ʼ����������ʼ��SPI1�͸�λI/O��������
  
  osal_start_reload_timer( SampleApp_TaskID, SAMPLEAPP_SEND_PERIODIC_MSG_EVT, SAMPLEAPP_SEND_PERIODIC_MSG_TIMEOUT ); //����5s��ʱ��
  
  osal_start_reload_timer( SampleApp_TaskID, SAMPLEAPP_WDT_PERIODIC_MSG_EVT, SAMPLEAPP_FEEDWDT_PERIODIC_TIMEOUT ); //����500ms��ʱ��
            
  WDT_Init();//���Ź���ʼ�����Ź�
  
#if !(defined (ZDO_COORDINATOR)) || !(defined (RTR_NWK)) //�ն�
  /* ��ʼ������ϵͳ������flash���� */
  if(SUCCESS != osal_nv_item_init(DEVICE_ID, DEVICE_ID_LEN, NULL)) //�豸ID����flash�����ʼ��
    printf("ID flash init failed!!\n");
  
  if(SUCCESS !=osal_nv_item_init(DEVICE_GH, DEVICE_GH_LEN, NULL)) //�豸��ϱ�ϵ������flash�����ʼ��
    printf("GH flash init failed!!\n");
  
  if(SUCCESS !=osal_nv_item_init(DEVICE_WDDT, DEVICE_WDDT_LEN, NULL)) //�豸�¶ȶ���ֵ����flash�����ʼ��
    printf("WDDT flash init failed!!\n");
  
  /* �ն˴���������ϵͳ��ʼ���������豸ID���ϱ�ϵ�� */
  if(SUCCESS == osal_nv_read(DEVICE_ID, 0, DEVICE_ID_LEN, dev_id)) //����flash�д�ŵ��豸ID
    sensor_id = atoi(dev_id);
  else
    printf("ID flash read failed!!\n");
  
  if(SUCCESS == osal_nv_read(DEVICE_GH, 0, DEVICE_GH_LEN, dev_gh)) //����flash�д�ŵ��豸��ϱ�ϵ��
    gh_factor = atof(dev_gh);
  else
    printf("GH flash read failed!!\n");
  
  if(SUCCESS == osal_nv_read(DEVICE_WDDT, 0, DEVICE_WDDT_LEN, dev_wddt)) //����flash�д�ŵ��豸�¶ȶ���ֵ
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
  
  // �����Ե�ͨѶ����
  Point_To_Point_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;//�㲥
  Point_To_Point_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  Point_To_Point_DstAddr.addr.shortAddr = 0x0000; //����Э����

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
 *          �¼������������е��¼�����������
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
        case CMD_SERIAL_MSG: //�����յ����ݺ��� MT_UART �㴫�ݹ��������ݣ������䷽�����գ�����ʱ������ MT������� 
          SampleApp_SerialCMD((mtOSALSerialData_t *)MSGpkt);
          break;

        // Received when a messages is received (OTA) for this endpoint
        case AF_INCOMING_MSG_CMD: //�������ݽ���
          SampleApp_MessageMSGCB( MSGpkt );
          break;

        // Received whenever the device changes state in the network
        case ZDO_STATE_CHANGE:
          SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if ( (SampleApp_NwkState == DEV_ZB_COORD)
              || (SampleApp_NwkState == DEV_ROUTER)
              || (SampleApp_NwkState == DEV_END_DEVICE) ) //�������ӳɹ�
          {
            printf("NET SUCCESS!\r\n");
            switch(SampleApp_NwkState)   //����_NIB�ṹ�е����ݵõ��豸�Ķ̵�ַ�����ڵ�̵�ַ���ŵ��ź�����PanID����ͨ�����ڴ�ӡ��ȥ
            {
            case DEV_ROUTER:  //·�����ڵ�
              printf("Router:0x%04X\r\nParent:0x%04X\r\nChannel:%d\r\nPanID:0x%04X\r\n",
                      _NIB.nwkDevAddress,_NIB.nwkCoordAddress,_NIB.nwkLogicalChannel,_NIB.nwkPanId);

              printf("IEEE:%02X%02X%02X%02X%02X%02X%02X%02X\r\n",      //IEEE��ַ
                      aExtendedAddress[7],aExtendedAddress[6],aExtendedAddress[5],aExtendedAddress[4],
                      aExtendedAddress[3],aExtendedAddress[2],aExtendedAddress[1],aExtendedAddress[0]);

              printf("Parent IEEE:%02X%02X%02X%02X%02X%02X%02X%02X\r\n",      //���ڵ�IEEE��ַ
                      _NIB.nwkCoordExtAddress[7],_NIB.nwkCoordExtAddress[6],_NIB.nwkCoordExtAddress[5],_NIB.nwkCoordExtAddress[4],
                      _NIB.nwkCoordExtAddress[3],_NIB.nwkCoordExtAddress[2],_NIB.nwkCoordExtAddress[1],_NIB.nwkCoordExtAddress[0]);

              break;
            case DEV_END_DEVICE: //�ն˽ڵ�
              printf("EndDevice:0x%04X\r\nParent:0x%04X\r\nChannel:%d\r\nPanID:0x%04X\r\n",
                      _NIB.nwkDevAddress,_NIB.nwkCoordAddress,_NIB.nwkLogicalChannel,_NIB.nwkPanId);

              printf("IEEE:%02X%02X%02X%02X%02X%02X%02X%02X\r\n",      //IEEE��ַ
                      aExtendedAddress[7],aExtendedAddress[6],aExtendedAddress[5],aExtendedAddress[4],
                      aExtendedAddress[3],aExtendedAddress[2],aExtendedAddress[1],aExtendedAddress[0]);

              printf("Parent IEEE:%02X%02X%02X%02X%02X%02X%02X%02X\r\n",      //���ڵ�IEEE��ַ
                      _NIB.nwkCoordExtAddress[7],_NIB.nwkCoordExtAddress[6],_NIB.nwkCoordExtAddress[5],_NIB.nwkCoordExtAddress[4],
                      _NIB.nwkCoordExtAddress[3],_NIB.nwkCoordExtAddress[2],_NIB.nwkCoordExtAddress[1],_NIB.nwkCoordExtAddress[0]);

              break;
            case DEV_ZB_COORD:  //Э����
              printf("Coordinator:0x%04X\r\nChannel:%d\r\nPanID:0x%04X\r\n",
                      _NIB.nwkDevAddress,_NIB.nwkLogicalChannel,_NIB.nwkPanId);

              printf("IEEE:%02X%02X%02X%02X%02X%02X%02X%02X\r\n",      //IEEE��ַ
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
            
#if (!defined (ZDO_COORDINATOR)) && (!defined (RTR_NWK))  //�ն˽ڵ�          
            orphan++;  //�����ۼ�
            if(orphan>ORPHAN_MAX)  //����ָ����������ʧ���������豸
            {
              rs_flag = 1; //ֹͣι������ʹϵͳ��λ
            }
#endif //�ն�
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
  /* AD�����¼������ն˴��������ݲɼ��ͷ�����Э���� */
  if ( events & SAMPLEAPP_ADC_MEA_EVT )
  {
    int numbers;
    static uint16 res_std;
    uint16 res_t0,res_u,res_gh;
    float temp;
    
    
 #if !(defined (ZDO_COORDINATOR)) && !(defined (RTR_NWK)) //�ն�
    /* �ն����ݲɼ����� */
    switch(adc_state)
    {
      case START: //��ʼ����
            ADC_MeasureStart(STDR_CHANNEL, 0, 3, TRUE); //��׼����ADת����ʼ    
            adc_state = READY1; //������һ״̬
            osal_start_timerEx( SampleApp_TaskID, SAMPLEAPP_ADC_MEA_EVT,SAMPLEAPP_ADC_MEA_TIMEOUT ); //200ms��ʱ���¼������ڵȴ�ADCת�����
            break;
            
      case READY1: //��׼����AD����ֵ��ȡ�������¶�ADCת��
            res_std = ADC_ReadData(0);
            ADC_MeasureStart(T0_CHANNEL, 0, 3, TRUE); //�¶�ADת����ʼ             
            adc_state = READY2; //������һ״̬
            osal_start_timerEx( SampleApp_TaskID, SAMPLEAPP_ADC_MEA_EVT,SAMPLEAPP_ADC_MEA_TIMEOUT ); //200ms��ʱ���¼������ڵȴ�ADCת�����
            break;
            
      case READY2: //�¶�AD����ֵ��ȡ������ʪ��ADCת��
            res_t0 = ADC_ReadData(0);
            ADC_MeasureStart(U_CHANNEL, 0, 0, TRUE); //ʪ��ADת����ʼ
            
            if(res_std==0) //��ֹ����Ϊ��
            {
              res_std = 1;
            }            
            temp = (float)res_t0/(float)res_std*100;
            temp = CalculateTemp(temp) + cal_t0 + CAL_T0_CONSTANT; //�����¶�У׼ֵ
            
            temp = temp*10; //�Ŵ�ʮ������������
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
            
            adc_state = READY3; //������һ״̬
            osal_start_timerEx( SampleApp_TaskID, SAMPLEAPP_ADC_MEA_EVT,SAMPLEAPP_ADC_MEA_TIMEOUT ); //200ms��ʱ���¼������ڵȴ�ADCת�����
            break;
            
      case READY3: //ʪ��AD����ֵ��ȡ�����������ADCת��
            res_u = ADC_ReadData(0);
            ADC_MeasureStart(GH_CHANNEL, 0, 3, TRUE); //�����ADת����ʼ ������8��          
            sensor.humidity = (float)res_u/65535*AD_VREF*100 + 0.5;
            
            if(debug)
            {
              printf("\r\nres_u=%u,humidity=%d\r\n", res_u, sensor.humidity);
            }
            
            adc_state = READY4; //������һ״̬
            osal_start_timerEx( SampleApp_TaskID, SAMPLEAPP_ADC_MEA_EVT,SAMPLEAPP_ADC_MEA_TIMEOUT ); //200ms��ʱ���¼������ڵȴ�ADCת�����
            break;
            
      case READY4: //�����AD����ֵ��ȡ��ת�����
            res_gh = ADC_ReadData(0);
            temp = (float)res_gh/65535*AD_VREF/8*1000; //��1000��ѹ��λת��ΪmV
            sensor.photosyn = CalculateRadiation(temp, gh_factor) + 0.5 + CAL_GH; //���Ϲ�����嶩��ֵ
            if(debug)
            {
              printf("\r\nres_gh=%u,guanghe=%d\r\n", res_gh, sensor.photosyn);
            }
            adc_done = TRUE;
            break;
    }
    
    if(adc_done) //����ת����ɣ����ݴ�����
    {
          
      adc_state = START; //��ʼ��
      
      adc_done = FALSE; //���㣬Ϊ�´�ת����׼��
      
      numbers = sprintf(data_sensor, "BG,ID%03u,%04d,%03d,%04d,ED\r\n", sensor_id, 
                        sensor.temperature, sensor.humidity, sensor.photosyn);
      
      /* �ն����ݷ��� */
      if(debug)
      {
        printf("\r\n%s", data_sensor);
      }
      
      SampleApp_SendPointToPointMessage(data_sensor, numbers); //��Ե㷢�ͺ���,�ն˴��������ݷ���Э����
    }
#endif

    // return unprocessed events
    return (events ^ SAMPLEAPP_ADC_MEA_EVT);
  }
  
  /* ���� 5s �¼����� */
  if( events & SAMPLEAPP_SEND_PERIODIC_MSG_EVT )
  {
#if !(defined (ZDO_COORDINATOR)) && !(defined (RTR_NWK)) //�ն�
    osal_set_event( SampleApp_TaskID, SAMPLEAPP_ADC_MEA_EVT );//����ADC���������¼�
#endif
    // return unprocessed events
    return (events ^ SAMPLEAPP_SEND_PERIODIC_MSG_EVT);
  }
  
  /* ����500msι���¼����� */
  if( events & SAMPLEAPP_WDT_PERIODIC_MSG_EVT )
  {
    if(!rs_flag)
    {
      WDT_Feed();//ι��
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
 * @brief   ��������������н��ܵ������ݣ��������������ĸ��豸�����Ǹ���
 *          ��ID���ֱ�ġ�
 *          ���н��յ������ݶ������ afIncomingMSGPacket_t *pkt��ָ��Ĵ洢����
 *          ��
 *          
 *          
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pkt ) //2017.03.17 ����棺ȥ����ID��ʽ������Ϣ��
{
  char *str = &pkt->cmd.Data[0];
  int len = pkt->cmd.DataLength;
  int sens_id = 0;
  char temp[20];
  int numbers;
  
  /* ���߽������ݴ�ӡ */
  if(debug) //������Ϣ
  {
    HalUARTWrite(0, str,len);//��ӡ�յ�����
  }
  
  /* ���߽������ݽ��� */
  if(*(str+len-1)== '\n')
  {
#if defined (ZDO_COORDINATOR) || defined (RTR_NWK) //Э����
    /* Э�����������ݽ��� */
    sens_id = atoi(str+DATA_OFFSET_ID);
    if(sens_id>=1&&sens_id<=3) //��ȡ����������
    {
      int i = 0;
      i = sens_id - 1;
      coordi[i].temperature = atoi(str+DATA_OFFSET_T0);
      coordi[i].humidity = atoi(str+DATA_OFFSET_U);
      coordi[i].photosyn = atoi(str+DATA_OFFSET_GH);
    }
    
#else //�ն�
    /* �ն˽������ݽ��� */
    if(osal_memcmp(str,"#RS",3))//��λ
    {
      if((len>5)&&(atoi(str+4)!=0))  //������Ϊ0�����������Ϣ
      {
        zgWriteStartupOptions(ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_NETWORK_STATE);
        printf("clear\r\n");
      }
      
      printf("reset\r\n");
      /* ��λ */     
      rs_flag = 1;//��λֹͣι����־     
    }
    else if(osal_memcmp(str,"#GH",3))
    {
      if(len == 5) //��ȡ��ϱ�ϵ��
      {
        numbers = sprintf(temp,"<GH,%03u,%.2f>\r\n", sensor_id, gh_factor);
        SampleApp_SendPointToPointMessage(temp, numbers);
      }
      else //���ù�ϱ�ϵ��
      {
        float temp_gh;
        int temp_id;
        char *p = NULL;
        
        p = strchr(str, ','); //���ҵ�һ��","���ֵ�λ��
        sscanf(p+1, "%d", &temp_id);
        if(temp_id == sensor_id)
        {
          p = strchr(p+1, ','); //���ҵڶ���","���ֵ�λ��
          sscanf(p+1, "%f", &temp_gh);
          if(temp_gh>=1.0)
          {
            gh_factor = temp_gh;
            
            /* ��ϱ�ϵ������flash */
            sprintf(dev_gh, "%5.2f", gh_factor);
            
            if(SUCCESS == osal_nv_write(DEVICE_GH, 0, DEVICE_GH_LEN, dev_gh))
              ReturnTorF(1,0); //����ɹ�
            else
              ReturnTorF(0,0); //����ʧ��
          }
          else
          {
            ReturnTorF(0,0); //����
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
 *          �����������һ�����������ݷ��ͺ����������豸����Ϊ�����Թ㲥��ʱ��
 *          ���õģ������������÷������ݵĵط�
 *   SAMPLEAPP_PERIODIC_CLUSTERID,  ����һ���أ�����������Ǻͽ��շ�������ϵ��
 *           Э�����յ������ţ������1������֤�����������Թ㲥��ʽ���͹�����
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_SendPeriodicMessage(char *p, int len)
{
  if ( AF_DataRequest( &SampleApp_Periodic_DstAddr, &SampleApp_epDesc,
                       SAMPLEAPP_PERIODIC_CLUSTERID,  
                       len,//�ֽ���
                       p,//ָ��ͷ
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
 * @brief   �����Ե㷢�ͺ���
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_SendPointToPointMessage( char *p, int len )
{ 
  if ( AF_DataRequest( &Point_To_Point_DstAddr,&SampleApp_epDesc,
                         SAMPLEAPP_POINT_TO_POINT_CLUSTERID,
                         len,//�ֽ���
                         p,//�׵�ַ  Ҫ�������ݵ��׵�ַ
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
 * @brief   ͨѶ���ڽ������ݽ���
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_SerialCMD(mtOSALSerialData_t *cmdMsg)
{
   uint8 i,len,*str=NULL; //len �������ݳ���
   int numbers;
   str=cmdMsg->msg; //ָ�����ݿ�ͷ
   len=*str; //msg ��ĵ� 1 ���ֽڴ����������ݳ���
   str++; //��ָ��ָ���һ���ַ�
   
   /********��ӡ�����ڽ��յ������ݣ�������ʾ*********/
   //HalUARTWrite(0,str,len);
   
   /* ��λ��ָ����� */
   if(*(str+len-1)== '\n')
   {
#if defined (ZDO_COORDINATOR) || defined (RTR_NWK) //Э����
     
      //���ݶ�ȡ��DM��
      if(osal_memcmp(str,"#DM",3)) //���ɶ�ȡ����
      {
        /* ���ɶ�ȡ���� */
        numbers = sprintf(data_coor, "BG,ID001,%04d,%03d,%04d,ID002,%04d,%03d,%04d,ID003,%04d,%03d,%04d,ED\r\n",coordi[0].temperature,
                          coordi[0].humidity,coordi[0].photosyn,coordi[1].temperature,coordi[1].humidity,coordi[1].photosyn,
                          coordi[2].temperature,coordi[2].humidity,coordi[2].photosyn); //�������
        
        SendDataToCollector(data_coor, numbers); //���ݷ�������
        
        osal_memset(coordi, 0, sizeof(dataType_t)*3); //���ͺ���������
      }
      
      //��λָ�RS��
      else if(osal_memcmp(str,"#RS",3))
      {
        /* ��λ */
        if((len>5)&&(atoi(str+4)!=0))  //������Ϊ0�����������Ϣ
        {
          zgWriteStartupOptions(ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_NETWORK_STATE);
          printf("clear\r\n");
        }
        
        ReturnTorF( 1, 1 ); //��ȷ
        SampleApp_SendPeriodicMessage(str, len); //ת����λָ����ն˴�����
        
        rs_flag = 1;//��λֹͣι����־
        
      }
      else if(osal_memcmp(str,"#GH",3)) //Э����ʹ��GHָ������������ù�ϱ�ϵ��
      {
        SampleApp_SendPeriodicMessage(str, len);
      }
#else //�ն˽ڵ�
      
      //�豸ID��ȡ�����ã�ID��
      if(osal_memcmp(str,"#ID",3))
      {
        if(len == 5) //��ȡ�豸ID
        {          
          printf("<ID,%03u>\r\n", sensor_id);
        }
        else //�����豸ID
        {
          int temp_id;
          
          sscanf(str+4, "%d", &temp_id);                           
          if(temp_id>=1 && temp_id<=3)
          {
            sensor_id = temp_id;
            
            /* �豸ID����flash */
            sprintf(dev_id, "%03u", sensor_id);
            
            if(SUCCESS == osal_nv_write(DEVICE_ID, 0, DEVICE_ID_LEN, dev_id))
              ReturnTorF( 1, 1 ); //����ɹ�
            else
              ReturnTorF( 0, 1 ); //����ʧ��
          }
          else
          {
            ReturnTorF( 0, 1 ); //����
          }
        }
      }
      
      //��ϱ��ȡ�����ã�GH��
      else if(osal_memcmp(str,"#GH",3))
      {
        if(len == 5) //��ȡ��ϱ�ϵ��
        {
          printf("<GH,%03u,%.2f>\r\n", sensor_id, gh_factor);
        }
        else //���ù�ϱ�ϵ��
        {
          float temp_gh;
          char *p = NULL;
          
          p = strchr(str, ','); //���ҵ�һ��","���ֵ�λ��
          p = strchr(p+1, ','); //���ҵڶ���","���ֵ�λ��
          sscanf(p+1, "%f", &temp_gh);
          if(temp_gh>=1.0)
          {
            gh_factor = temp_gh;
            
            /* ��ϱ�ϵ������flash */
            sprintf(dev_gh, "%5.2f", gh_factor);
            
            if(SUCCESS == osal_nv_write(DEVICE_GH, 0, DEVICE_GH_LEN, dev_gh))
              ReturnTorF( 1, 1 ); //����ɹ�
            else
              ReturnTorF( 0, 1 ); //����ʧ��
          }
          else
          {
            ReturnTorF( 0, 1 ); //����
          }
        }
      }
      
      //��λָ�RS��
      else if(osal_memcmp(str,"#RS",3)) 
      {
        /* ��λ */
        if((len>5)&&(atoi(str+4)!=0))  //������Ϊ0�����������Ϣ
        {
          zgWriteStartupOptions(ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_NETWORK_STATE);
          printf("clear\r\n");
        }
        ReturnTorF( 1, 1 );//��λ�ɹ�
        rs_flag = 1;//��λֹͣι����־
      }
      
      //�¶ȶ�����CR��
      else if(osal_memcmp(str,"#CR",3)) //�¶ȶ���
      {
        if( len == 5 ) //��ȡ�¶ȶ���ֵ
        {
          printf("<CR,%.2f>\r\n", cal_t0);
        }
        else if( len > 5 )
        {
          float temp_wddt;
          char *p = NULL;
            
          p = strchr(str, ','); //���ҵ�һ��","���ֵ�λ��
          sscanf( p+1, "%f", &temp_wddt);
          if( temp_wddt>-10.0 && temp_wddt<10.00 )
          {
            cal_t0 = temp_wddt;
             /* �¶ȶ�������flash */
            sprintf(dev_wddt, "%5.2f", cal_t0);
            
            if(SUCCESS == osal_nv_write(DEVICE_WDDT, 0, DEVICE_WDDT_LEN, dev_wddt))
              ReturnTorF( 1, 1 ); //����ɹ�
            else
              ReturnTorF( 0, 1 ); //����ʧ��
            
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
      //��ѯ������PANID��PD��
      else if(osal_memcmp(str,"#PD",3))
      {
        if(len==5)  //��ȡ
        {
          printf("<PD,0x%04X>\r\n", _NIB.nwkPanId);
        }
        else if(len>5)  //����
        {
          uint16 panid=0;
          
          sscanf(str+4,"%x",&panid);  //�õ��µ�PanID
          _NIB.nwkPanId=panid;  //����PanID
          
          /* 2017.4.10���ʹ��NV_INIT��NV_RESTOREʱ����channel��panidʱͬʱ������ڵ㱣���Э����������Ϣ��
             ����������ڸ���channel��panid���������ͬchannel��panid������Э������������������Э�������޷�������
             ��������Ҫ�����ڵ���ܼ��������磬����PANID����ʱ��Ҫ��Ϊ0xFFFF��channelҪѡ��ȫ��11-26ͨ�� */
          _NIB.nwkCoordAddress = INVALID_NODE_ADDR;
          osal_memset( _NIB.nwkCoordExtAddress, 0, Z_EXTADDR_LEN );
          /* */
          
          NLME_UpdateNV(0x01);  //�� NIB ��Ϣ���� NV �У�������ʹ���µ�PanID
          
          ReturnTorF( 1, 1 );
        }
        else
        {
          ReturnTorF( 0, 1 );
        }
      }
      
      //��ѯ�����ŵ��ţ�CH��
      else if(osal_memcmp(str,"#CH",3))
      {
        if(len==5)  //��ȡ
        {
          printf("<CH,0x%02X>\r\n", _NIB.nwkLogicalChannel);
        }
        else if(len>5)  //����
        {
          uint16 channel=0;
          
          sscanf(str+4,"%x",&channel);  //�õ��µ�channel
          _NIB.nwkLogicalChannel=channel;  //����channel
          
          /* 2017.4.10���ʹ��NV_INIT��NV_RESTOREʱ����channel��panidʱͬʱ������ڵ㱣���Э����������Ϣ��
             ����������ڸ���channel��panid���������ͬchannel��panid������Э������������������Э�������޷�������
             ��������Ҫ�����ڵ���ܼ��������磬����PANID����ʱ��Ҫ��Ϊ0xFFFF��channelҪѡ��ȫ��11-26ͨ�� */
          _NIB.nwkCoordAddress = INVALID_NODE_ADDR;
          osal_memset( _NIB.nwkCoordExtAddress, 0, Z_EXTADDR_LEN );
          /* */
          
          NLME_UpdateNV(0x01);  //�� NIB ��Ϣ���� NV �У�������Ӧ��
          
          ReturnTorF( 1, 1 );
        }
        else
        {
          ReturnTorF( 0, 1 );
        }
      }
      
      //��ѯ�̵�ַ��SA��
      else if(osal_memcmp(str,"#SA",3))
      {
        printf("<SA,0x%04X>\r\n", _NIB.nwkDevAddress);
      }
      
      /* 2017.4.10����޸�����ʱĬ�ϵ�chennel��panid������NV_INIT��NV_RESTORE��
         ���ն˽ڵ���ӣ�Э��������ʹ��NV_INIT��NV_RESTORE���ֲ��䣬��ΪЭ������Ҫ����������Ϣ���ھӱ�ȣ�
         �����ն˽ڵ������������ַ����Ϣ����ı� */
      /* 16.8.5���DCH��CPD�����ȡ������Ĭ�������ŵ���PANID������ԭ����CH��PD���������鿴��ǰ������ŵ���PANID */
      //��ѯ������Ĭ������PANID��DPD��
      else if(osal_memcmp(str,"#DPD",3))
      {
        if(len==6)  //��ȡ
        {
          printf("<DPD,0x%04X>\r\n", zgConfigPANID);   /* Ĭ��PANID */
        }
        else if(len>6)  //����
        {
          uint16 panid=0;
          
          sscanf(str+5,"%x",&panid);  //�õ��µ�PanID
          
          extern uint8 nvItemInit( uint16 id, uint16 len, void *buf, uint8 set );
          zgConfigPANID = panid;   /* �޸�Ĭ��panid */
          //���浽NV
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
      
      
      //��ѯ����Ĭ�������ŵ��ţ�DCH��
      else if(osal_memcmp(str,"#DCH",4))
      {
        if(len==6)  //��ȡ
        {
          if(zgDefaultChannelList == MAX_CHANNELS_24GHZ)  //ͨ��ȫѡ
          {
            printf("<DCH,ALL>\r\n");
          }
          else
          {
            for(i=11;i<=26;i++)  //����channel list�õ��ŵ����ŵ�����ʱֻѡһ������ȫѡ
            {
              if(zgDefaultChannelList&((uint32)1<<i)) break;
            }
            printf("<DCH,0x%02X>\r\n", i);
          }
        }
        else if(len>6)  //����
        {
          uint16 channel=0;
          
          sscanf(str+5,"%x",&channel);  //�õ��µ�channel
          
          /* 2017.4.10����ŵ�����Ϊ"ALL"��"all"ʱ��ʾͨ��ȫѡ����ʱ��channel��Ϊ0��ʾͨ��ȫѡ */
          if(osal_memcmp(str+5,"ALL",3) || osal_memcmp(str+5,"all",3))  //11-26ͨ��ȫѡ
          {
            channel=0;
          }
          
          extern uint8 nvItemInit( uint16 id, uint16 len, void *buf, uint8 set );
          if(((channel<11) || (channel>26)) && (channel!=0))  //��Ч�ŵ��Ҳ�Ϊ0
          {
            printf("<CH,F>\r\n");
          }
          else
          {
            //����channel�õ�channel list��ע��ǿ������ת��
            if(channel==0)  //Ϊ0ʱ���ŵ�ȫѡ
            {
              zgDefaultChannelList=MAX_CHANNELS_24GHZ;
            }
            else
            {
              zgDefaultChannelList=(uint32)((uint32)1<<channel);
            }
            printf("<CH,0x%02X,0x%08lX>\r\n", channel, zgDefaultChannelList);
            //���浽NV
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
      
      //������Ϣ���أ�DG��
      else if(osal_memcmp(str,"#DG",3)) 
      {
        int temp;
        
        temp = atoi(str+4);
        if(temp==0||temp==1)
        {
          debug = temp;
          ReturnTorF( 1, 1 ); //��ȷ
        }
        else
        {
          ReturnTorF( 0, 1 ); //����
        }
        
      }
      else
      {
        ReturnTorF( 0, 1 ); //����
      }
   }
   else
   {
      ReturnTorF( 0, 1 ); //����
   }
   
}

/*********************************************************************
 * @fn      sendData
 *
 * @brief   ������ɼ�����������
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
 * @brief   Э�����򴮿�ָ�����,flag��1 ָʾ�ն��豸��0 ָʾЭ�����豸
 *
 * @param   none
 *
 * @return  none
 */
//void Order_Parse(char *str, int len, int flag)
//{
//   uint8 i;
//   int numbers;
//     /* ��λ��ָ����� */
//   if(*(str+len-1)== '\n')
//   {
//#if defined (ZDO_COORDINATOR) || defined (RTR_NWK) //Э����
//      if(osal_memcmp(str,"#DM",3)) //���ɶ�ȡ����
//      {
//        /* ���ɶ�ȡ���� */
//        numbers = sprintf(data_coor, "BG,ID001,%04d,%03d,%04d,ID002,%04d,%03d,%04d,ID003,%04d,%03d,%04d,ED\r\n",coordi[0].temperature,
//                          coordi[0].humidity,coordi[0].photosyn,coordi[1].temperature,coordi[1].humidity,coordi[1].photosyn,
//                          coordi[2].temperature,coordi[2].humidity,coordi[2].photosyn); //�������
//        
//        SendDataToCollector(data_coor, COOR_DATA_LEN); //���ݷ������ɼ���
//        
//        osal_memset(coordi, 0, sizeof(dataType_t)*3); //���ͺ���������
//      }
//      else if(osal_memcmp(str,"#RS",3))
//      {
//        /* ��λ */
//        ReturnTorF(1, 1); //��ȷ
//        SampleApp_SendPeriodicMessage(str, len); //ת����λָ����ն˴�����
//        
//        rs_flag = 1;//��λֹͣι����־
//        
//      }
//      else if(osal_memcmp(str,"#GH",3)) //Э����ʹ��GHָ������������ù�ϱ�ϵ��
//      {
//        SampleApp_SendPeriodicMessage(str, len);
//      }
//#else //�ն˽ڵ�
//      if(osal_memcmp(str,"#ID",3))
//      {
//        if(len == 5) //��ȡ�豸ID
//        {          
//          printf("<ID,%03u>\r\n", sensor_id);
//        }
//        else //�����豸ID
//        {
//          int temp_id;
//          
//          sscanf(str+4, "%d", &temp_id);                           
//          if(temp_id>=1 && temp_id<=3)
//          {
//            sensor_id = temp_id;
//            
//            /* �豸ID����flash */
//            sprintf(dev_id, "%03u", sensor_id);
//            
//            if(SUCCESS == osal_nv_write(DEVICE_ID, 0, DEVICE_ID_LEN, dev_id))
//              HalUARTWrite(0,"T\n",2 ); //����ɹ�
//            else
//              HalUARTWrite(0,"F\n",2 ); //����ʧ��
//          }
//          else
//          {
//            HalUARTWrite(0,"F\n",2 ); //����
//          }
//        }
//      }
//      else if(osal_memcmp(str,"#GH",3))
//      {
//        if(len == 5) //��ȡ��ϱ�ϵ��
//        {
//          printf("<GH,%05.2f>\r\n", gh_factor);
//        }
//        else //���ù�ϱ�ϵ��
//        {
//          float temp_gh;
//          
//          sscanf(str+4, "%f", &temp_gh);
//          if(temp_gh>=1.0)
//          {
//            gh_factor = temp_gh;
//            
//            /* ��ϱ�ϵ������flash */
//            sprintf(dev_gh, "%5.2f", gh_factor);
//            
//            if(SUCCESS == osal_nv_write(DEVICE_GH, 0, DEVICE_GH_LEN, dev_gh))
//              HalUARTWrite(0,"T\n",2 ); //����ɹ�
//            else
//              HalUARTWrite(0,"F\n",2 ); //����ʧ��
//          }
//          else
//          {
//            HalUARTWrite(0,"F\n",2 ); //����
//          }
//        }
//      }
//      else if(osal_memcmp(str,"#RS",3))//��λ
//      {
//        /* ��λ */     
//        rs_flag = 1;//��λֹͣι����־     
//      }
//#endif
//      else if(osal_memcmp(str,"#DG",3)) //������Ϣ����
//      {
//        int temp;
//        
//        temp = atoi(str+4);
//        if(temp==0||temp==1)
//        {
//          debug = temp;
//          HalUARTWrite(0,"T\n",2 ); //��ȷ
//        }
//        else
//        {
//          HalUARTWrite(0,"F\n",2 ); //����
//        }
//        
//      }
//      else
//      {
//        HalUARTWrite(0,"F\n",2 ); //����
//      }
//   }
//   else
//   {
//      HalUARTWrite(0,"F\n",2 ); //����
//   }
//}

/*********************************************************************
 * @fn      ReturnTorF
 *
 * @brief   ָ���'T'��'F'������a:1 �ظ�'T',0 �ظ�'F'������b:1 �������ڣ�0 ����Э����
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
