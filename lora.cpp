#include "lora.h"
#include "elevator_protocol_parse.h"
#include "mainwindow.h"
#include "transport_crc.h"
#include "ui_mainwindow.h"

extern transport_t lora_transport;

struct LORA_DATA lora_data;
struct ELEVATOR ele_tx;
QByteArray lora_rx;
QByteArray lora_tx;

void lora_rx_frame_parse(transport_t *transport, uint8_t *frame, int32_t length) {
    lora_rx.clear();
    for (int i = 0; i < length; i++) {
        lora_rx.push_back(frame[i]);
    }
}

int32_t lora_tx_frame(intptr_t handle, uint8_t *data, int32_t length) {
    lora_tx.clear();
    for (int i = 0; i < length; i++) {
        lora_tx.push_back(data[i]);
    }
    // qDebug() << "lora_tx: " << lora_tx.toHex().toUpper();
    return 0;
}

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

/* enable or disable ui group */
void MainWindow::send_cmd_enabled(bool value) { ui->send_cmd->setEnabled(value); }

void MainWindow::elevator_send_config_enabled(bool value) {
    ui->val_process_id->setEnabled(value);
    ui->val_message_id->setEnabled(value);
    ui->val_ele_id->setEnabled(value);
    ui->val_robot_id->setEnabled(value);
    /* save config parameter */
    if (!value) {
        ele_tx.processId      = ui->val_process_id->text().toUInt();
        ele_tx.messageId      = ui->val_message_id->text().toUInt();
        ele_tx.eleId          = ui->val_ele_id->text().toInt();
        ele_tx.robotId        = ui->val_robot_id->text().toInt();
        ele_tx.lora.direction = Robot;
    }
    /**/
    else {
    }
}

/* enable or disable send elevator config */
void MainWindow::on_save_send_config_clicked(bool checked) {
    if (checked) {
        elevator_send_config_enabled(false);
        send_cmd_enabled(true);
    } else {
        elevator_send_config_enabled(true);
        send_cmd_enabled(false);
    }
}

/* send custom data by lora */
void MainWindow::lora_send_user_data(QString str) {
    ui->logBrowser->insertPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") + " [send] " +
                                    "\r\n" + str + "\r\n");
    /* 自动滚动进度条 */
    ui->logBrowser->moveCursor(QTextCursor::End);
}

uint8_t elevator_send_buf[64];

/* elevator config send QByteArray */
void elevator_config_send_buf(void) {
    memset(&elevator_send_buf, 0, sizeof(elevator_send_buf));
    elevator_send_buf[0]  = ELEVATOR_PROTOCOL_VERSION;
    elevator_send_buf[1]  = (uint8_t)ele_tx.processId;
    elevator_send_buf[2]  = (uint8_t)(ele_tx.processId >> 8);
    elevator_send_buf[3]  = (uint8_t)(ele_tx.processId >> 16);
    elevator_send_buf[4]  = (uint8_t)(ele_tx.processId >> 24);
    elevator_send_buf[5]  = (uint8_t)ele_tx.messageId;
    elevator_send_buf[6]  = (uint8_t)(ele_tx.messageId >> 8);
    elevator_send_buf[7]  = (uint8_t)(ele_tx.messageId >> 16);
    elevator_send_buf[8]  = (uint8_t)(ele_tx.messageId >> 24);
    elevator_send_buf[9]  = (uint8_t)ele_tx.eleId;
    elevator_send_buf[10] = (uint8_t)(ele_tx.eleId >> 8);
    elevator_send_buf[11] = ((Robot << 4) | ele_tx.robotId);
    // qDebug() << ((elevator_send_buf[10] << 8) | (elevator_send_buf[9]));
}

void MainWindow::on_preempt_elevator_button_clicked() {
    elevator_config_send_buf();
    elevator_send_buf[12] = ELEVATOR_CMD;
    elevator_send_buf[13] = PREEMPT_ELEVATOR;
    elevator_send_buf[14] = 0x00;
    elevator_send_buf[15] = 0x00;
    elevator_send_buf[16] = 0x00;
    lora_send(elevator_send_buf, 17);
    lora_data.send_buf = "发送(机器):" + QString::number((ele_tx.robotId)) + "号机器人抢占电梯.";
    /* send lora_tx buf */
    this->serialPort->write(lora_tx);
    lora_send_user_data(lora_data.send_buf);
}

void MainWindow::on_call_elevator_button_clicked() {
    elevator_config_send_buf();
    elevator_send_buf[12] = ELEVATOR_CMD;
    elevator_send_buf[13] = EXTERNAL_CALL;
    elevator_send_buf[14] = 0x00;
    elevator_send_buf[15] = 0x02;
    elevator_send_buf[16] = 0x00;
    elevator_send_buf[17] = ui->val_floor->text().toInt();
    elevator_send_buf[18] = ui->val_door_time->text().toInt();
    lora_send(elevator_send_buf, 19);
    lora_data.send_buf =
        "发送(机器):外呼楼层:" + ui->val_floor->text() + ",开门时间:" + ui->val_door_time->text() + "秒.";
    /* send lora_tx buf */
    this->serialPort->write(lora_tx);
    lora_send_user_data(lora_data.send_buf);
}

void MainWindow::on_query_elevator_status_button_clicked() {
    elevator_config_send_buf();
    elevator_send_buf[12] = QUERY;
    elevator_send_buf[13] = QUERY_ELEVATOR_STATUS;
    elevator_send_buf[14] = 0x00;
    elevator_send_buf[15] = 0x00;
    elevator_send_buf[16] = 0x00;
    lora_send(elevator_send_buf, 17);
    lora_data.send_buf = "发送(机器):查询电梯状态.";
    /* send lora_tx buf */
    this->serialPort->write(lora_tx);
    lora_send_user_data(lora_data.send_buf);
}

void MainWindow::on_open_door_button_clicked() {
    elevator_config_send_buf();
    elevator_send_buf[12] = ELEVATOR_CMD;
    elevator_send_buf[13] = CONTROL_DOOR;
    elevator_send_buf[14] = 0x00;
    elevator_send_buf[15] = 0x03;
    elevator_send_buf[16] = 0x00;
    elevator_send_buf[17] = 0x00;
    elevator_send_buf[18] = 0x01; /* open door */
    elevator_send_buf[19] = ui->val_door_time->text().toInt();
    lora_send(elevator_send_buf, 20);
    lora_data.send_buf = "发送(机器):请求电梯开门" + ui->val_door_time->text() + "秒.";
    /* send lora_tx buf */
    this->serialPort->write(lora_tx);
    lora_send_user_data(lora_data.send_buf);
}

void MainWindow::on_close_door_button_clicked() {
    elevator_config_send_buf();
    elevator_send_buf[12] = ELEVATOR_CMD;
    elevator_send_buf[13] = CONTROL_DOOR;
    elevator_send_buf[14] = 0x00;
    elevator_send_buf[15] = 0x03;
    elevator_send_buf[16] = 0x00;
    elevator_send_buf[17] = 0x00;
    elevator_send_buf[18] = 0x00; /* open door */
    elevator_send_buf[19] = 0x00;
    lora_send(elevator_send_buf, 20);
    lora_data.send_buf = "发送(机器):请求电梯关门.";
    /* send lora_tx buf */
    this->serialPort->write(lora_tx);
    lora_send_user_data(lora_data.send_buf);
}

void MainWindow::on_release_elevator_button_clicked() {
    elevator_config_send_buf();
    elevator_send_buf[12] = ELEVATOR_CMD;
    elevator_send_buf[13] = RELEASE_ELEVATOR;
    elevator_send_buf[14] = 0x00;
    elevator_send_buf[15] = 0x00;
    elevator_send_buf[16] = 0x00;
    lora_send(elevator_send_buf, 17);
    lora_data.send_buf = "发送(机器):" + QString::number((ele_tx.robotId)) + "号机器人释放电梯.";
    /* send lora_tx buf */
    this->serialPort->write(lora_tx);
    lora_send_user_data(lora_data.send_buf);
}

void MainWindow::on_cancel_elevator_task_button_clicked() {
    elevator_config_send_buf();
    elevator_send_buf[12] = ELEVATOR_CMD;
    elevator_send_buf[13] = CANCEL_TASK;
    elevator_send_buf[14] = 0x00;
    elevator_send_buf[15] = 0x00;
    elevator_send_buf[16] = 0x00;
    lora_send(elevator_send_buf, 17);
    lora_data.send_buf = "发送(机器):" + QString::number((ele_tx.robotId)) + "号机器人取消任务.";
    /* send lora_tx buf */
    this->serialPort->write(lora_tx);
    lora_send_user_data(lora_data.send_buf);
}

void MainWindow::on_reset_elevator_button_clicked(){
    elevator_config_send_buf();
    elevator_send_buf[12] = ELEVATOR_CMD;
    elevator_send_buf[13] = ELEVATOR_REBOOT;
    elevator_send_buf[14] = 0x00;
    elevator_send_buf[15] = 0x00;
    elevator_send_buf[16] = 0x00;
    lora_send(elevator_send_buf, 17);
    lora_data.send_buf = "发送(机器):请求电梯复位.";
    /* send lora_tx buf */
    this->serialPort->write(lora_tx);
    lora_send_user_data(lora_data.send_buf);
}