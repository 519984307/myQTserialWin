#ifndef LORA_H
#define LORA_H

#include <mainwindow.h>
#include "transport_crc.h"

#define ELEVATOR_PROTOCOL_VERSION (3)

/* elevator cmd pos not for transport */
#define ELEVATOR_VERSION_POS (2)
#define ELEVATOR_PROCESS_ID_POS (3)
#define ELEVATOR_MESSAGE_ID_POS (7)
#define ELEVATOR_ID_POS (11)
#define ELEVATOR_DIRECTION (13)
#define ELEVATOR_COMMAND (14)
#define ELEVATOR_TODO (15)
#define ELEVATOR_FLOOR_POS (51)
#define ELEVATOR_RUN_STATUS_POS (55)
#define ELEVATOR_ARRIVED_STATUS (58)

/* elevator cmd pos for transport */

/* lora send and recv buf */
struct LORA_DATA {
    QString recv_buf;
    QString send_buf;
};

/* lora_parse */
void lora_rx_frame_parse(transport_t *transport, uint8_t *frame, int32_t length);
void lora_send(uint8_t *data, uint16_t len);
int32_t lora_tx_frame(intptr_t handle, uint8_t *data, int32_t length);

#endif  // LORA_H
