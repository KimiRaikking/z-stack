/**************************************************************************************************
  Filename:       LocationApp.c
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

#include "LocationApp.h"
#include "LocationAppHw.h"

#include "OnBoard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_keypad.h"

/* CALCULATE */
#include "math.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// This list should be filled with Application specific Cluster IDs. �ǰe�ʥ]��ID,�]�tPeriodic clusterID��flash clusterID
const cId_t SampleApp_ClusterList[SAMPLEAPP_MAX_CLUSTERS] =
{
  SAMPLEAPP_PERIODIC_CLUSTERID,   // periodic clusterID = 1
  SAMPLEAPP_FLASH_CLUSTERID       // flash clusterID = 2
};

const SimpleDescriptionFormat_t SampleApp_SimpleDesc =
{
  SAMPLEAPP_ENDPOINT,              //  int Endpoint;
  SAMPLEAPP_PROFID,                //  uint16 AppProfId[2];
  SAMPLEAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  SAMPLEAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  SAMPLEAPP_FLAGS,                 //  int   AppFlags:4;          //SAMPLEAPP_ENDPOINT ~ SAMPLEAPP_FLAGS:���ε{��������ID(���U�h�ӻ�)
  SAMPLEAPP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)SampleApp_ClusterList,  //  uint8 *pAppInClusterList;
  SAMPLEAPP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)SampleApp_ClusterList   //  uint8 *pAppInClusterList;  //MAX_CLUSTERS ~ SampleApp_ClusterList:����&�ǰe���ʥ]ID���A�����Υu�ǰe�W�z���ʥ]ID
};                                                              

endPointDesc_t SampleApp_epDesc;   //����y�z�����Ϊ���Ƶ��c

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
                          // This variable will be received when SampleApp_Init() is called.
devStates_t SampleApp_NwkState;

uint8 SampleApp_TransID;  // This is the unique message ID (counter)

afAddrType_t SampleApp_Periodic_DstAddr;
afAddrType_t SampleApp_Flash_DstAddr;

aps_Group_t SampleApp_Group;

uint8 SampleAppPeriodicCounter = 0;
uint8 SampleAppFlashCounter = 0;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void SampleApp_HandleKeys( uint8 shift, uint8 keys );
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void SampleApp_SendPeriodicMessage( void );
void SampleApp_SendFlashMessage( uint16 flashTime ); 
void SampleApp_LocationMSG(afIncomingMSGPacket_t *msg);

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
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SampleApp_Init( uint8 task_id )
{
  SampleApp_TaskID = task_id;
  SampleApp_NwkState = DEV_INIT; // Device state : Initialized - not connected to anything
  SampleApp_TransID = 0; // The unique message ID (counter)

#if defined ( HOLD_AUTO_START ) // HOLD_AUTO_START�sĶ�ﶵ�|����ZDApp���Ұʸ˸m�����ε{��
  // HOLD_AUTO_START is a compile option that will surpress ZDApp from starting the device 
  // and wait for the application to start the device.
  ZDOInitDevice(0);
#endif
  
  /* SampleApp_Periodic Message's */
  // Setup for the periodic message's destination address Broadcast to everyone
  SampleApp_Periodic_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast; // �ǰe�Ҧ�-�s��
  SampleApp_Periodic_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;          // end-point�s��:20
  SampleApp_Periodic_DstAddr.addr.shortAddr = 0xFFFF;                // ������}(0xFFFF -> �s����������Ҧ��˸m(�t��v))

  /* SampleApp_Flash Command's */
  // Setup for the flash command's destination address - Group 1
  SampleApp_Flash_DstAddr.addrMode = (afAddrMode_t)afAddrGroup;      // �ǰe�Ҧ�-�s�զ�}
  SampleApp_Flash_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;             // end-point�s��:20
  SampleApp_Flash_DstAddr.addr.shortAddr = SAMPLEAPP_FLASH_GROUP;    // �s�զW�٦�}
  
  /* SampleApp_epDesc description */
  // Fill out the endpoint description.
  SampleApp_epDesc.endPoint = SAMPLEAPP_ENDPOINT;                    // SampleApp�y�z��endpoint�s��
  SampleApp_epDesc.task_id = &SampleApp_TaskID;                      // SampleApp�y�z��TaskID
  SampleApp_epDesc.simpleDesc = (SimpleDescriptionFormat_t *)&SampleApp_SimpleDesc;   // SampleApp�y�z���Ÿ�
  SampleApp_epDesc.latencyReq = noLatencyReqs;                       // �bAF�h���U���end-point

  // Register the endpoint description with the AF(�V�U�h���U�����ε{��)
  afRegister( &SampleApp_epDesc );

  // Register for all key events - This app will handle all key events(�V�U�h���U���s�ƥ�)
  RegisterForKeys( SampleApp_TaskID );

  // By default, all devices start out in Group 1 //���U�s��
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
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SampleApp_TaskID ); // �z�Losal_msg_receive function ����SampleApp_TaskID�лx
    while ( MSGpkt )
    {    
      switch ( MSGpkt->hdr.event )
      {
        case KEY_CHANGE: // Received when a key is pressed(�����Ukey�ɶi�汵��)
          SampleApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case AF_INCOMING_MSG_CMD: // Received when a messages is received (OTA) for this endpoint(��endpoint������message�i�汵��)
          SampleApp_MessageMSGCB( MSGpkt );
          break;

        case ZDO_STATE_CHANGE: // Received whenever the device changes state in the network(�[�J�����᪬�A���ܮɶi�汵��)
          SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if ((SampleApp_NwkState == DEV_ZB_COORD) || (SampleApp_NwkState == DEV_ROUTER) || (SampleApp_NwkState == DEV_END_DEVICE) )
          {
            // Start sending the periodic message in a regular interval. �}�l�b�w�����ɶ����j�ǰe�w���T��
            osal_start_timerEx( SampleApp_TaskID, SAMPLEAPP_SEND_PERIODIC_MSG_EVT, SAMPLEAPP_SEND_PERIODIC_MSG_TIMEOUT );
          }
          else
            { /* Device is no longer in the network(device���b��������) */ }
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

  // Send a message out - This event is generated by a timer (setup in SampleApp_Init()).
  if ( events & SAMPLEAPP_SEND_PERIODIC_MSG_EVT )
  {
    // Send the periodic message
    SampleApp_SendPeriodicMessage();
    // Setup to send message again in normal period (+ a little jitter)
    osal_start_timerEx( SampleApp_TaskID, 
                        SAMPLEAPP_SEND_PERIODIC_MSG_EVT,
                       (SAMPLEAPP_SEND_PERIODIC_MSG_TIMEOUT + (osal_rand() & 0x00FF)) );

    // return unprocessed events
    return (events ^ SAMPLEAPP_SEND_PERIODIC_MSG_EVT);
  }
  // Discard unknown events
  return 0;
}

/*********************************************************************
 * Event Generation Functions
 */
/*********************************************************************
 * @fn      SampleApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
void SampleApp_HandleKeys( uint8 shift, uint8 keys )
{
  (void)shift;  // Intentionally unreferenced parameter
  
  if ( keys & HAL_KEY_SW_1 )
  {
    /* This key sends the Flash Command is sent to Group 1. This device will not
       receive the Flash Command from this device (even if it belongs to group 1). */
    SampleApp_SendFlashMessage( SAMPLEAPP_FLASH_DURATION ); //SAMPLEAPP_FLASH_DURATION -> 1000 milliseconds
  }

  if ( keys & HAL_KEY_SW_2 )
  {
    /* The Flashr Command is sent to Group 1.
     * This key toggles this device in and out of group 1.
     * If this device doesn't belong to group 1, this application
     * will not receive the Flash command sent to group 1. */
    aps_Group_t *grp;
    grp = aps_FindGroup( SAMPLEAPP_ENDPOINT, SAMPLEAPP_FLASH_GROUP );
    if ( grp )
    {
      // Remove from the group
      aps_RemoveGroup( SAMPLEAPP_ENDPOINT, SAMPLEAPP_FLASH_GROUP );
    }
    else
    {
      // Add to the flash group
      aps_AddGroup( SAMPLEAPP_ENDPOINT, &SampleApp_Group );
    }
  }
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      SampleApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  uint16 flashTime;
  
  switch ( pkt->clusterId )
  {
    // �g���ʰT��
    case SAMPLEAPP_PERIODIC_CLUSTERID:  
          SampleApp_LocationMSG(pkt); // Get the Location Message 
      break;
    
    // LED Blink �T��
    case SAMPLEAPP_FLASH_CLUSTERID: 
      flashTime = BUILD_UINT16(pkt->cmd.Data[1], pkt->cmd.Data[2] );
      HalLedBlink( HAL_LED_4, 4, 50, (flashTime / 4) ); //Blink the LED4
      break;
  }
}

/*********************************************************************
 * @fn      SampleApp_SendPeriodicMessage
 *
 * @brief   Send the periodic message.
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_SendPeriodicMessage( void )
{
 
  if ( AF_DataRequest ( &SampleApp_Periodic_DstAddr, &SampleApp_epDesc,
                       SAMPLEAPP_PERIODIC_CLUSTERID,
                       1,
                       (uint8*)&SampleAppPeriodicCounter,
                       &SampleApp_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS
     )
  { /* Success occurred in request to send*/ }
  else
  { /* Error occurred in request to send. */ }
}

/*********************************************************************
 * @fn      SampleApp_SendFlashMessage
 *
 * @brief   Send the flash message to group 1.
 *
 * @param   flashTime - in milliseconds
 *
 * @return  none
 */
void SampleApp_SendFlashMessage( uint16 flashTime )
{
  uint8 buffer[3];
  buffer[0] = (uint8)(SampleAppFlashCounter++);
  buffer[1] = LO_UINT16( flashTime );
  buffer[2] = HI_UINT16( flashTime );
  
  if ( AF_DataRequest               
       ( &SampleApp_Flash_DstAddr,     // �ت��a��} 
         &SampleApp_epDesc,            // end-point�y�z�Ÿ�
         SAMPLEAPP_FLASH_CLUSTERID,    // Cluster ID
         3,                            // MessageData�ƾڪ���
         buffer,                       // MessageData�ƾڥ[��
         &SampleApp_TransID,           // �T��ID
         AF_DISCV_ROUTE,               
         AF_DEFAULT_RADIUS             // �o�e�b�|
       ) == afStatus_SUCCESS
     )
  { /* occurred in request to send. */ }
  else
  { /* Error occurred in request to send. */ }
}

/*********************************************************************
*********************************************************************/

/*********************************************************************
 * @fn      SampleApp_LocationMSG
 *
 * @brief   Calculate Reference Node short address, LQI and RSSI value
 *
 * @return  none
 */
void SampleApp_LocationMSG(afIncomingMSGPacket_t *msg)
{
  //Location variable
  static uint16 shaddr;
  static uint16 addr_a,addr_b,addr_c;
  static uint8 LQI_a,LQI_b,LQI_c;
  static int8 RSSI_a,RSSI_b,RSSI_c;
  static double Distance_a,Distance_b,Distance_c;
  static int flag_a,flag_b,flag_c;
  
  //Location Algorithm calculate variable
  static double cos_R,sin_R; 
  static int Point_x,Point_y,Loc_X,Loc_Y; // The Location Loc_X and Loc_Y
  
  //Distance Parameter setting
  float n = 3.75; // n is signal propagation constant, also named propagation exponent.
  int A = 47; // A is received signal strength at a distance of one meter.
  
  int num = 31087; // coordinator receiver short address
//  int num = 5168;
  
  if (SampleApp_NwkState == DEV_ZB_COORD)
  {
//  if(SampleApp_NwkState == DEV_ROUTER)
//  {
      shaddr = msg->srcAddr.addr.shortAddr; // get the short address
      
      if(shaddr == num)
         {
            addr_a = shaddr;
            LQI_a = msg->LinkQuality;  //get the LQI value in the Packet frame
            RSSI_a = -(81-(LQI_a*91) / 255); //LQI value change to the RSSI value
            halLcdDisplayUint16(HAL_LCD_LINE_1, 3, HAL_LCD_RADIX_DEC, shaddr);
            halLcdDisplayUint8(HAL_LCD_LINE_1, 11, HAL_LCD_RADIX_DEC, RSSI_a);
//            Distance_a = pow(10,((ABS(RSSI_a)- A)/(10*n))); // RSSI value change to Distance value
//            flag_a = 1;
        }
        if(shaddr == num + 1)
          {
            addr_b = shaddr;
            LQI_b = msg->LinkQuality;  //get the LQI value in the Packet frame
            RSSI_b = -(81-(LQI_b*91) / 255); //LQI value change to the RSSI value
            halLcdDisplayUint16(HAL_LCD_LINE_2, 3, HAL_LCD_RADIX_DEC, shaddr);
            halLcdDisplayUint8(HAL_LCD_LINE_2, 11, HAL_LCD_RADIX_DEC, RSSI_b);
//            Distance_b = pow(10,((ABS(RSSI_b)- A)/(10*n))); // RSSI value change to Distance value
//            flag_b = 1;
          }
          if(shaddr == num + 2)
            {
              addr_c = shaddr;
              LQI_c = msg->LinkQuality;  //get the LQI value in the Packet frame
              RSSI_c = -(81-(LQI_c*91) / 255); //LQI value change to the RSSI value
              halLcdDisplayUint16(HAL_LCD_LINE_3, 3, HAL_LCD_RADIX_DEC, shaddr);
              halLcdDisplayUint8(HAL_LCD_LINE_3, 11, HAL_LCD_RADIX_DEC, RSSI_c);
//              Distance_c = pow(10,((ABS(RSSI_c)- A)/(10*n))); // RSSI value change to Distance value
//              //flag_c = 1;
            }
//            if(flag_a == 1 && flag_b == 1)
//              {
//                cos_R = ((((double)Distance_a*(double)Distance_a)) + (((double)Distance_b)*((double)Distance_b)) 
//                         - (10.0*10.0)) / (2.0*((double)Distance_a)*((double)Distance_b));
//                sin_R = sqrt(1.0 - (((double)cos_R)*((double)cos_R)));
//          
//                Point_x = (int) (Distance_a * sin_R);
//                Point_y = (int) (Distance_a * cos_R);
//          
//                Loc_X = -((255-Point_x)+1);
//                Loc_Y = -((255-Point_y)+1);
//                
//                halLcdDisplayUint8(HAL_LCD_LINE_4, 3, HAL_LCD_RADIX_DEC, Loc_X);
//                halLcdDisplayUint8(HAL_LCD_LINE_4, 11, HAL_LCD_RADIX_DEC, Loc_Y);              
//                flag_a = 0; flag_b = 0;
//              }
//              halLcdDisplayUint16(HAL_LCD_LINE_2, 3, HAL_LCD_RADIX_DEC, addr_a);
//              halLcdDisplayUint8(HAL_LCD_LINE_2, 12, HAL_LCD_RADIX_DEC, (uint8)Distance_a);
//              halLcdDisplayUint16(HAL_LCD_LINE_3, 3, HAL_LCD_RADIX_DEC, addr_b);
//              halLcdDisplayUint8(HAL_LCD_LINE_3, 12, HAL_LCD_RADIX_DEC, (uint8)Distance_b);
  }
//  }
}