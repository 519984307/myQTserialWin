#include "transport_crc.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MINPACKLEN 6
#define P_HEADER 0xAA
#define P_TAIL 0x55
#define P_CTRL 0xA5
#define P_FAIL 0
#define P_SUCCESS 1

/**
 * @brief  crc16_ibm
 * @param  data
 * @param  len
 * @retval crc
 */
uint16_t crc16_ibm_step(uint16_t crc, uint8_t *data, int32_t length) {
  uint8_t i;
  // uint16_t crc = 0xffff;        // Initial value
  while (length--) {
    crc ^= *data++; // crc ^= *data; data++;
    for (i = 0; i < 8; ++i) {
      if (crc & 1)
        crc = (crc >> 1) ^ 0xA001; // 0xA001 = reverse 0x8005
      else
        crc = (crc >> 1);
    }
  }
  return crc;
}

/**
 * @brief  crc16_ibm
 * @param  data
 * @param  len
 * @retval crc
 */
uint16_t crc16_ibm(uint8_t *data, int32_t length) {
  return crc16_ibm_step(0xffff, data, length);
}

/**
 * @brief  generate a trans frame
 * @param  data
 * @param  len
 * @retval result
 */
int32_t generateFrame(uint8_t *dataIn, int32_t dataInLen, uint8_t *dataOut,
                      int32_t *dataOutLen) {
  int32_t i = 0;
  int32_t j = 0;
  int32_t maxOutLen = 0;
  uint16_t crc = 0;
  uint8_t crcu8[2];
  uint8_t data;

  maxOutLen = *dataOutLen;

  if (!dataIn || !dataOut || maxOutLen < dataInLen + MINPACKLEN) {
    return -1;
  }

  dataOut[i++] = P_HEADER;
  dataOut[i++] = P_HEADER;

  crc = crc16_ibm(dataIn, dataInLen);
  crcu8[0] = (uint8_t)(crc & 0xff);
  crcu8[1] = (uint8_t)(crc >> 8);

  for (j = 0; j < dataInLen; j++) {
    data = dataIn[j];
    if (data == P_HEADER || data == P_TAIL || data == P_CTRL) {
      if (i >= maxOutLen) {
        return -2;
      }
      dataOut[i++] = P_CTRL;
    }
    if (i >= maxOutLen) {
      return -2;
    }
    dataOut[i++] = data;
  }

  for (j = 0; j < 2; j++) {
    data = crcu8[j];
    if (data == P_HEADER || data == P_TAIL || data == P_CTRL) {
      if (i >= maxOutLen) {
        return -2;
      }
      dataOut[i++] = P_CTRL;
    }
    if (i >= maxOutLen) {
      return -2;
    }
    dataOut[i++] = data;
  }

  if (i >= maxOutLen + 2) {
    return -2;
  }
  dataOut[i++] = P_TAIL;
  dataOut[i++] = P_TAIL;
  *dataOutLen = i;
  return 0;
}

int32_t getDataFromFrame(uint8_t *dataIn, int32_t dataInLen, uint8_t *dataOut,
                         uint32_t *dataOutLen) {
  int j = 0;
  for (int i = 2; i < dataInLen - 2; i++, j++) {
    if (dataIn[i] == P_CTRL) {
      dataOut[j] = dataIn[++i];
    } else {
      dataOut[j] = dataIn[i];
    }
  }
  *dataOutLen = j - 2;
  if (*(int *)dataOutLen < 0) {
    return -2;
  }

  uint16_t crc = crc16_ibm(dataOut, *dataOutLen);

  uint16_t crc_receive = dataOut[*dataOutLen] + (dataOut[*dataOutLen + 1] << 8);

  if (crc == crc_receive) {
    return 0;
  } else {
    return -1;
  }
}

int32_t sendFrame(transport_t *transport, uint8_t *frame, int32_t length) {
  uint8_t dataOut[TRANS_BUFF_LEN];
  int32_t ret = 0;
  uint32_t dataOutLen = sizeof(dataOut) / sizeof(dataOut[0]);

  if (!transport) {
    return -1;
  }

  ret = generateFrame(frame, length, dataOut, (int32_t *)&dataOutLen);
  if (ret < 0) {
    return ret;
  }

  // printLayerHex("transport layer send", dataOut, dataOutLen);
  ret = transport->send(transport->handle, dataOut, dataOutLen);
  return ret;
}

void receiveByte(transport_t *transport, uint8_t byte) {
  frameState_t state = transport->state;
  uint16_t len = transport->len;
  uint8_t frameData[RECV_BUFF_LEN];
  uint32_t frameDataLen = 0;
  int32_t crcCheck;

  if (!transport) {
    return;
  }

  if (len >= sizeof(transport->buf) / sizeof(transport->buf[0])) {
    len = 0;
    state = FRAME_STATE_IDLE;
  }

  switch (state) {
  case FRAME_STATE_IDLE:
    switch (byte) {
    case P_HEADER:
      transport->buf[len++] = byte;
      state = FRAME_STATE_FIRST_HEADER;
      break;
    case P_TAIL:
    default:
      break;
    }
    break;
  case FRAME_STATE_FIRST_HEADER:
    switch (byte) {
    case P_HEADER:
      transport->buf[len++] = byte;
      state = FRAME_STATE_SECOND_HEADER;
      break;
    case P_TAIL:
    default:
      len = 0;
      state = FRAME_STATE_IDLE;
      break;
    }
    break;
  case FRAME_STATE_SECOND_HEADER:
    switch (byte) {
    case P_HEADER:
      state = FRAME_STATE_SECOND_HEADER;
      break;
    case P_TAIL:
      transport->buf[len++] = byte;
      state = FRAME_STATE_FIRST_TAIL;
      break;
    default:
      transport->buf[len++] = byte;
      state = FRAME_STATE_FRAME_DATA;
      break;
    }
    break;
  case FRAME_STATE_FRAME_DATA:
    switch (byte) {
    case P_HEADER:
      transport->buf[len++] = byte;
      state = FRAME_STATE_HEADER_IN_DATA;
      break;
    case P_TAIL:
      transport->buf[len++] = byte;
      state = FRAME_STATE_FIRST_TAIL;
      break;
    default:
      transport->buf[len++] = byte;
      break;
    }
    break;
  case FRAME_STATE_FIRST_TAIL:
    switch (byte) {
    case P_HEADER:
      transport->buf[len++] = byte;
      state = FRAME_STATE_HEADER_IN_DATA;
      break;
    case P_TAIL:
      transport->buf[len++] = byte;
      crcCheck =
          getDataFromFrame(transport->buf, len, frameData, &frameDataLen);
      if (0 == crcCheck) {
        if (transport->onFrameDetected) {
          transport->onFrameDetected(transport, frameData, frameDataLen);
        }
      } else if (-1 == crcCheck) {
        if (transport->onFrameDetected) {
          transport->onFrameDetected(transport, frameData,
                                     frameDataLen | 0xFFFF0000);
        }
      } else {
        // printf("data invalid ! len<=0 \n");
      }

      len = 0;
      state = FRAME_STATE_IDLE;
      break;
    default:
      transport->buf[len++] = byte;
      state = FRAME_STATE_FRAME_DATA;
      break;
    }
    break;
  case FRAME_STATE_HEADER_IN_DATA:
    switch (byte) {
    case P_HEADER:
      len = 0;
      transport->buf[len++] = byte;
      transport->buf[len++] = byte;
      state = FRAME_STATE_SECOND_HEADER;
      break;
    case P_TAIL:
      transport->buf[len++] = byte;
      state = FRAME_STATE_FIRST_TAIL;
      break;
    default:
      transport->buf[len++] = byte;
      state = FRAME_STATE_FRAME_DATA;
      break;
    }
    break;
  default:
    break;
  }

  transport->state = state;
  transport->len = len;
  //     printLayerHex("transport buf", transport->buf, len);
}

int32_t initTransportCRC(transport_t *transport) {
  if (!transport) {
    return -1;
  }
  transport->sendFrame = sendFrame;
  transport->generateFrame = generateFrame;
  transport->receiveByte = receiveByte;
  return 0;
}
