#ifndef TRANSPORTCRC_H
#define TRANSPORTCRC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "transport.h"
#include <stdint.h>

typedef struct {
  uint8_t *dataInBuf;
  int32_t dataInLen;
  uint8_t *dataOutBuf;
  int32_t *dataOutLen;
} data_frame_t;

extern int32_t initTransportCRC(transport_t *transport);
extern void printLayerHex(const char *title, uint8_t *buf, uint32_t len);
uint16_t crc16_ibm(uint8_t *data, int32_t length);
uint16_t crc16_ibm_step(uint16_t crc, uint8_t *data, int32_t length);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TRANSPORTCRC_H */
