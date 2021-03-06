#ifndef ZCL_SAMPLESW_H
#define ZCL_SAMPLESW_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "zcl.h"
#include "hal_sensor.h"

#define ZIGBEEDEVICE_ENDPOINT           12
#define ZIGBEEDEVICE_MAX_ATTRIBUTES     11

#define LIGHT_OFF                       0x00
#define LIGHT_ON                        0x01

// Send Message Timeout
#define GENERICAPP_SEND_MSG_TIMEOUT     3000  // Every 3 seconds
//#define SEND_DISPLAY_TIMER              5     // every 5 min sec
#define SEND_DISPLAY_TIMER              5000     // every 5 min sec  
  
// Application Events (OSAL)
#define SAMPLESW_IDENTIFY_TIMEOUT_EVT   0x0001 // Events for the sample app
#define SENSOR_SEND_MSG_EVT             0x0002 // These are bit weighted definitions.
#define RS485_MSG_EVT                   0x0004 // These is UART/RS485 event , Chris Uart Evt
#define KEYPAD_MSG_EVT                  0x0008

#define GENERICAPP_CLUSTERID            1 
  
#if defined ( TRANSMITAPP_FRAGMENTED )
#define TRANSMITAPP_MAX_DATA_LEN    225
#else
#define TRANSMITAPP_MAX_DATA_LEN    102
#endif  

extern SimpleDescriptionFormat_t zclZigbeeDevice_SimpleDesc;
extern CONST zclAttrRec_t zclZigbeeDevice_Attrs[];
extern uint8  zclSampleSw_OnOff;
extern uint16 zclSampleSw_IdentifyTime;
extern byte TransmitApp_Msg[ TRANSMITAPP_MAX_DATA_LEN ]; // ZCL transmit msg variable
extern afAddrType_t zclZigbeeDevice_DstAddr;

extern byte zclZigbeeDevice_TaskID;
extern void zcl_ZigbeeDevice_Init( byte task_id );
extern UINT16 zclZigbeeDevice_event_loop( byte task_id, UINT16 events );
extern void zclRS485_CallBack(uint8 port, uint8 event); // The RS485 Message callback

#ifdef __cplusplus
}
#endif

#endif /* ZCL_SAMPLEAPP_H */
