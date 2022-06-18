#ifndef _TRANSPORT_H
#define _TRANSPORT_H
#include <stdint.h>

#define TRANS_BUFF_LEN 256
#define RECV_BUFF_LEN 512

typedef enum frameState {
  FRAME_STATE_IDLE,
  FRAME_STATE_FIRST_HEADER,
  FRAME_STATE_SECOND_HEADER,
  FRAME_STATE_FRAME_DATA,
  FRAME_STATE_FIRST_TAIL,
  FRAME_STATE_HEADER_IN_DATA
} frameState_t;

typedef struct Transport transport_t;
typedef struct Transport {
  intptr_t handle;
  frameState_t state;
  uint8_t buf[RECV_BUFF_LEN];
  uint16_t len;

  void *uplayer;
  void (*onFrameDetected)(transport_t *transport, uint8_t *frame,
                          int32_t length);
  int32_t (*send)(intptr_t handle, uint8_t *data, int32_t length);

  int32_t (*sendFrame)(transport_t *transport, uint8_t *frame, int32_t length);
  int32_t (*generateFrame)(uint8_t *dataIn, int32_t dataInLen, uint8_t *dataOut,
                           int32_t *dataOutLen);
  void (*receiveByte)(transport_t *transport, uint8_t byte);

} transport_t;

#endif /* _TRANSPORT_H */
