#include "lora.h"
#include "mainwindow.h"
#include "transport_crc.h"
#include "ui_mainwindow.h"

extern transport_t lora_transport;

void lora_rx_frame_parse(transport_t *transport, uint8_t *frame,
                         int32_t length) {
  QString rec_buf, out_buf;
  for (int i = 0; i < length; i++) {
    if (i == length - 1) {
      rec_buf = QString::asprintf("%02X", frame[i]);
    } else {
      rec_buf = QString::asprintf("%02X ", frame[i]);
    }
    // QTextStream(stdout) << rec_buf;
    out_buf += rec_buf;
  }
  qDebug() << out_buf;
  //  ui->logBrowser->insertPlainText(
  //      QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
  //      " [receive] " + "\r\n" + rec_buf + "\r\n");
  // QTextStream(stdout) << "\n";
}

int32_t lora_tx_frame(intptr_t handle, uint8_t *data, int32_t length) {
  return 0;
}

void lora_send(uint8_t *data, uint16_t len) {
  lora_transport.sendFrame(&lora_transport, data, len);
}

QString MainWindow::lora_signal(QString str) {
  QString floor, run_status, arrived, signal, ret;
  int signal_val, floor_val;
  static int i = 0;
  /* use regular expression to match the signal check */
  QRegularExpression re("1[0-9] 7C 20 00 03");
  /* match the slave reply */
  if (re.match(str).hasMatch()) {
    floor = "楼层:";
    floor += str.mid(ELEVATOR_FLOOR_POS, 2) + " ";
    floor_val = str.mid(ELEVATOR_FLOOR_POS, 2).toInt();
    run_status = "运动状态:";
    run_status += str.mid(ELEVATOR_RUN_STATUS_POS, 1) + " ";
    arrived = "到达状态:";
    arrived += str.mid(ELEVATOR_ARRIVED_STATUS, 1) + " ";
    signal = str.mid(ELEVATOR_SIGNAL_CHECK_POS, 2);
    /* 16进制字符串转10进制 */
    signal_val = signal.toInt(NULL, 16) / 2;
    this->LoraSignalQualitySeries->append(i, -signal_val);
    this->floorChangedSeries->append(i++, floor_val);
    signal = "信号强度:-";
    signal += QString::number(signal_val) + "dBm";
    ret = floor + run_status + arrived + signal;
    //    qDebug() << "found heart packet";
    //    qDebug() << ret;
  }
  return ret;
}
