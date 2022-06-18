#include "mainwindow.h"
#include "lora.h"
#include "sys.h"
#include "transport_crc.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  this->serialPort = new QSerialPort;
  findFreeports();
  /* detect the toggled signal(这是一种自检测，不连接其他槽) */
  connect(ui->openCom, &QCheckBox::toggled, [=](bool checked) {
    /* the check is TRUE */
    if (checked) {
      initSerialPort();
      ui->send->setEnabled(true);
      /* 关闭串口配置 */
      ui->portName->setDisabled(true);
      ui->baudRate->setDisabled(true);
      ui->parity->setDisabled(true);
      ui->dataBits->setDisabled(true);
      ui->stopBits->setDisabled(true);
    } else {
      /* the check is FALSE */
      this->serialPort->close();
      ui->send->setEnabled(false);
      /* check false status */
      ui->openCom->setChecked(false);
    }
  });
  /* 接收数据连接(手动连接) */
  // connect(this->serialPort, SIGNAL(readyRead()), this, SLOT(recvMsg()));
  connect(this->serialPort, &QSerialPort::readyRead, this,
          &MainWindow::recvMsg);
  /* 发送数据连接 */
  connect(ui->send, &QPushButton::clicked,
          [=]() { sendMsg(ui->message->toPlainText()); });
}

/* 主窗口的析构函数 */
MainWindow::~MainWindow() { delete ui; }

/* find free serial ports */
void MainWindow::findFreeports() {
  QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
  /* 遍历所有可用的串口 */
  for (int i = 0; i < ports.size(); i++) {
    if (ports.at(i).isBusy()) {
      ports.removeAt(i);
      continue;
    }
    ui->portName->addItem(ports.at(i).portName());
    /* 显示具体的串口信息 */
    qDebug() << "PortName:" << ports.at(i).portName();
    qDebug() << "Description:" << ports.at(i).description();
  }
  /* 如果没有可用的串口 */
  if (!ports.size()) {
    QMessageBox::warning(NULL, "Tips", QStringLiteral("找不到空闲串口!"));
    return;
  }
}

/* init serial config */
bool MainWindow::initSerialPort() {
  this->serialPort->setPortName(ui->portName->currentText());
  /* 尝试打开串口，如果打开失败返回 */
  if (!this->serialPort->open(QIODevice::ReadWrite)) {
    QMessageBox::warning(NULL, "Tip", QStringLiteral("串口打开失败"));
    return false;
  }
  /* set baudRate */
  this->serialPort->setBaudRate(ui->baudRate->currentText().toInt());

  /* set parity bit */
  if (ui->parity->currentText() == "NONE") {
    this->serialPort->setParity(QSerialPort::NoParity);
  } else if (ui->parity->currentText() == "ODD") {
    this->serialPort->setParity(QSerialPort::OddParity);
  } else if (ui->parity->currentText() == "EVEN") {
    this->serialPort->setParity(QSerialPort::EvenParity);
  }

  /* set lora serial M0、M1 pin LOW */
  this->serialPort->setRequestToSend(TRUE);
  this->serialPort->setDataTerminalReady(TRUE);

  /* set data size */
  if (ui->dataBits->currentText().toInt() == DATA_BITS_8) {
    this->serialPort->setDataBits(QSerialPort::Data8);
  } else if (ui->dataBits->currentText().toInt() == DATA_BITS_7) {
    this->serialPort->setDataBits(QSerialPort::Data7);
  } else if (ui->dataBits->currentText().toInt() == DATA_BITS_6) {
    this->serialPort->setDataBits(QSerialPort::Data6);
  } else if (ui->dataBits->currentText().toInt() == DATA_BITS_5) {
    this->serialPort->setDataBits(QSerialPort::Data5);
  }

  /* set stop bits */
  if (ui->stopBits->currentText().toInt() == STOP_BITS_1) {
    this->serialPort->setStopBits(QSerialPort::OneStop);
  } else if (ui->stopBits->currentText().toInt() == STOP_BITS_2) {
    this->serialPort->setStopBits(QSerialPort::TwoStop);
  }
  return true;
}

/* send message to serial port */
void MainWindow::sendMsg(const QString &msg) {
  this->serialPort->write(QByteArray::fromHex(msg.toLatin1()));
  ui->logBrowser->insertPlainText(
      QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
      " [send] " + "\r\n" + msg + "\r\n");
}

extern transport_t lora_transport;

/* receive message from serial port */
void MainWindow::recvMsg() {
  QByteArray msg = this->serialPort->readAll();
  /* 将接收到的数据通过串口显示到log区域 */
  if (!msg.isEmpty()) {
    /* 使用ascii码格式接收 */
    if (ui->rx_ascii->isChecked()) {
      ui->logBrowser->insertPlainText(
          QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") +
          " [receive] " + "\r\n" + msg + "\r\n");
    }
    /* 使用16进制格式进行解析 */
    else if (ui->rx_hex->isChecked()) {
      QString rec_buf;
      for (int i = 0; i < msg.count(); i++) {
        QString str;
        /* 以16进制输出，但还是使用拼接成字符串的方式 */
        if (i == msg.count() - 1) {
          /* 最后一个字符串不输出空格 */
          str = QString::asprintf("%02X", (unsigned char)msg.at(i));
        } else {
          str = QString::asprintf("%02X ", (unsigned char)msg.at(i));
        }
        rec_buf += str;
      }

      /* lora信号质量检测 */
      if (ui->signal_quality->isChecked()) {
        rec_buf = lora_signal(rec_buf);
      }
      /* 打开乘梯模式 */
      else if (ui->elevator_mode->isChecked()) {
        for (int i = 0; i < msg.count(); i++) {
          lora_transport.receiveByte(&lora_transport, (uint8_t)msg.at(i));
        }
      }
      if (!rec_buf.isEmpty()) {
        ui->logBrowser->insertPlainText(
            QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") +
            " [receive] " + "\r\n" + rec_buf + "\r\n");
      }
    }
    /* 自动滚动进度条 */
    ui->logBrowser->moveCursor(QTextCursor::End);
  }
}

/* button clear RX log */
void MainWindow::on_clearRxLog_clicked() { ui->logBrowser->clear(); }

/* button clear TX log */
void MainWindow::on_clearTxLog_clicked() { ui->message->clear(); }

/* save data to file(not continuous) */
void MainWindow::on_saveFile_clicked(bool checked) {
  if (checked) {
    if (ui->logBrowser->toPlainText().isEmpty()) {
      QMessageBox::information(this, "提示消息", tr("没有数据需要保存！"),
                               QMessageBox::Ok);
      return;
    }
    QString fileName =
        QFileDialog::getSaveFileName(this, tr("保存为"), tr("未命名.txt"));
    QFile file(fileName);
    /* 如果用户取消了保存则直接退出函数 */
    if (file.fileName().isEmpty()) {
      return;
    }
    /* 如果打开失败则给出提示并退出函数 */
    if (!file.open(QFile::WriteOnly | QIODevice::Text)) {
      QMessageBox::warning(this, tr("保存文件"),
                           tr("打开文件 %1 失败, 无法保存\n%2")
                               .arg(fileName)
                               .arg(file.errorString()),
                           QMessageBox::Ok);
      return;
    }
    /* 写数据到文件 */
    QTextStream out(&file);
    out << ui->logBrowser->toPlainText();
    file.close();
  }
}
