#include "lora.h"
#include "mainwindow.h"
#include "transport_crc.h"
#include "ui_mainwindow.h"

extern transport_t lora_transport;

struct LORA_DATA lora_data;
QByteArray lora_recv;

void lora_rx_frame_parse(transport_t *transport, uint8_t *frame, int32_t length) {
    lora_recv.clear();
    for (int i = 0; i < length; i++) {
        lora_recv.push_back(frame[i]);
    }
}

int32_t lora_tx_frame(intptr_t handle, uint8_t *data, int32_t length) { return 0; }

void lora_send(uint8_t *data, uint16_t len) { lora_transport.sendFrame(&lora_transport, data, len); }

QString MainWindow::lora_signal(QString str, QString signal, int &i) {
    QString floor, run_status, arrived, ret;
    int floor_val;
    floor = "楼层:";
    floor += str.mid(ELEVATOR_FLOOR_POS, 2) + " ";
    floor_val  = str.mid(ELEVATOR_FLOOR_POS, 2).toInt();
    run_status = "运动状态:";
    run_status += str.mid(ELEVATOR_RUN_STATUS_POS, 1) + " ";
    arrived = "到达状态:";
    arrived += str.mid(ELEVATOR_ARRIVED_STATUS, 1) + " ";
    this->floorChangedSeries->append(i++, floor_val);
    ret = floor + run_status + arrived + signal;
    // qDebug() << "found heart packet";
    // qDebug() << ret;
    return ret;
}
