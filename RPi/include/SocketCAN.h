#ifndef SOCKETCAN_H
#define SOCKETCAN_H

#include <stdint.h>

/* The maximum number of objects of type SocketCAN_T allowed */
#ifndef MAX_SOCKETCAN_T
#define MAX_SOCKETCAN_T     1U 
#endif

/*Special address description flags for CAN_ID*/
#define CAN_EFF_FLAG 0x80000000U
#define CAN_RTR_FLAG 0x40000000U
#define CAN_ERR_FLAG 0x20000000U

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
    BITRATE_125000KBS = 125000,
    BITRATE_500000KBS = 500000,
    BITRATE_1000000MBS = 1000000
}SocketCAN_Bitrate_T;

typedef enum {
    IFACE_CAN0,
    IFACE_CAN1,
    /* --- */
    SocketCAN_InterfaceNumber_MAX
}SocketCAN_InterfaceNumber_T;

typedef struct{
    SocketCAN_InterfaceNumber_T iface_number;
    SocketCAN_Bitrate_T bitrate;
} SocketCAN_Config_T;

typedef struct{
    uint32_t can_id;    /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    uint8_t can_dlc;    /* frame payload length in byte (0 .. 8) */
    uint8_t data[8];    /* Data to send */
}SocketCAN_FrameConfig_T;

typedef struct{
    uint32_t can_id;    /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    uint32_t can_mask;    /* frame payload length in byte (0 .. 8) */
    size_t length;
}SocketCAN_FrameFilter_T;

/* First Class ADT */
typedef struct SocketCAN_T* SocketCAN_T_ptr;

SocketCAN_T_ptr SocketCAN_Create(const SocketCAN_Config_T* const config);
int SocketCAN_SetBitrate(SocketCAN_T_ptr const can_iface);
int SocketCAN_GetBitrate(const SocketCAN_T_ptr const can_iface);
int SocketCAN_SendMessage(SocketCAN_T_ptr const can_iface, const SocketCAN_FrameConfig_T* const frame_config);
int SocketCAN_ReadMessage(SocketCAN_T_ptr const can_iface);
int SocketCAN_SetFrameFilter(SocketCAN_T_ptr const can_iface, SocketCAN_FrameFilter_T* frame_filter);
int SocketCAN_Destroy(const SocketCAN_T_ptr const can_iface);

#ifdef __cplusplus
}
#endif 

#endif // SOCKETCAN_H