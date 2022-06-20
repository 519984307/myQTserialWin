#ifndef LORA_H
#define LORA_H

#include "transport_crc.h"
#include <mainwindow.h>

/* lora parse macro*/
/* QUERY 7C*/
#define QUERY (124)
#define ELEVATOR_STATUS (32)
#define HEART_PACKET (43)
#define SLAVE_OTA (37)
#define SLAVE_OTA_DONE (38)
/* CONFIG 7D */
#define ELEVATOR_CONFIG (125)
/* COMMAND 7E */
/* TODO */
#define ELEVATOR_CMD (126)
#define EXTERNAL_CALL (32)
#define CONTROL_DOOR (34)
#define CANCEL_TASK (35)
#define INTERNAL_CALL (40)
#define PREEMPT_ELEVATOR (41)
#define RELEASE_ELEVATOR (48)

/* elevator cmd pos not for transport */
#define ELEVATOR_VERSION_POS (2)
#define ELEVATOR_PROCESS_ID_POS (3)
#define ELEVATOR_MESSAGE_ID_POS (7)
#define ELEVATOR_ID_POS (11)
#define ELEVATOR_DIRECTION (13)
#define ELEVATOR_COMMAND (14)
#define ELEVATOR_TODO (15)
#define ELEVATOR_FLOOR_POS (57)
#define ELEVATOR_RUN_STATUS_POS (61)
#define ELEVATOR_ARRIVED_STATUS (64)
#define ELEVATOR_SIGNAL_CHECK_POS (78)

/* elevator cmd pos for transport */

/* lora_parse */
void lora_rx_frame_parse(transport_t *transport, uint8_t *frame,
                         int32_t length);
int32_t lora_tx_frame(intptr_t handle, uint8_t *data, int32_t length);

// QString lora_signal(QString str);

#endif // LORA_H
