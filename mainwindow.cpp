#include "mainwindow.h"
#include "elevator_protocol_parse.h"
#include "lora.h"
#include "sys.h"
#include "transport_crc.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->serialPort              = new QSerialPort;
    this->LoraSignalQualitySeries = new QtCharts::QLineSeries();
    this->floorChangedSeries      = new QtCharts::QLineSeries();
    findFreeports();
    /* detect the toggled signal(这是一种自检测，不连接其他槽) */
    connect(ui->openCom, &QCheckBox::toggled, [=](bool checked) {
        /* the check is TRUE */
        if (checked) {
            initSerialPort();
            ui->send->setEnabled(true);
            /* 关闭串口配置 */
            serial_config_disable(true);
            /* 默认开启乘梯模式 */
            // ui->elevator_mode->setChecked(true);
            /* clear data */
            LoraSignalQualitySeries->clear();
            floorChangedSeries->clear();
        } else {
            /* the check is FALSE */
            this->serialPort->close();
            ui->send->setEnabled(false);
            /* check false status */
            ui->openCom->setChecked(false);
            /* 打开串口配置 */
            serial_config_disable(false);
            /* 关闭乘梯模式 */
            // ui->elevator_mode->setChecked(false);
        }
    });
    /* 接收数据连接(手动连接) */
    connect(this->serialPort, &QSerialPort::readyRead, this, &MainWindow::recvMsg);
    /* 发送数据连接 */
    connect(ui->send, &QPushButton::clicked, [=]() { sendMsg(ui->message->toPlainText()); });
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
    ui->logBrowser->insertPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " [send] " + "\r\n" +
                                    msg + "\r\n");
}

void MainWindow::serial_config_disable(bool value) {
    /* 串口配置使能，失能 */
    ui->portName->setDisabled(value);
    ui->baudRate->setDisabled(value);
    ui->parity->setDisabled(value);
    ui->dataBits->setDisabled(value);
    ui->stopBits->setDisabled(value);
}

extern transport_t lora_transport;
extern LORA_DATA lora_data;
extern struct ELEVATOR ele;
extern QByteArray lora_recv;

/* receive message from serial port */
void MainWindow::recvMsg() {
    QByteArray msg = this->serialPort->readAll();
    /* 将接收到的数据通过串口显示到log区域 */
    if (!msg.isEmpty()) {
        /* 使用ascii码格式接收 */
        if (ui->rx_ascii->isChecked()) {
            ui->logBrowser->insertPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") +
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
            qDebug() << rec_buf;
            /* 乘梯模式数据解析*/
            if (ui->elevator_mode->isChecked()) {
                /* 使用transpostCRC协议解析原始数据 */
                for (int i = 0; i < msg.count(); i++) {
                    lora_transport.receiveByte(&lora_transport, (uint8_t)msg.at(i));
                }
                // msg = lora_recv;
                qDebug() << lora_recv.size();
                // rec_buf = lora_data.recv_buf;
                rec_buf = elevator_lora_data_parse(lora_recv);
            }
            /* lora信号质量检测开启 */
            else if (ui->signal_quality->isChecked()) {
                QString signal;
                int signal_val;
                static int i = 0;
                /* use regular expression to match the signal check */
                QRegularExpression re("1[0-9] 7C 20 00 03");
                if (re.match(rec_buf).hasMatch()) {
                    /* 提取lora信号强度 */
                    static QString lora_siganl_quality;
                    auto siganl_quality_pos = rec_buf.lastIndexOf(" ");
                    lora_siganl_quality     = rec_buf.mid(siganl_quality_pos + 1, 2);
                    signal_val              = lora_siganl_quality.toUInt(NULL, 16) / 2;
                    this->LoraSignalQualitySeries->append(i++, -signal_val);
                    signal = "信号强度:-";
                    signal += QString::number(signal_val) + "dBm";
                    // qDebug() << lora_siganl_quality;
                    /* 使用transportCRC协议原始协议 */
                    for (int i = 0; i < msg.count(); i++) {
                        lora_transport.receiveByte(&lora_transport, (uint8_t)msg.at(i));
                    }
                    rec_buf = lora_data.recv_buf;
                    // qDebug() << rec_buf;
                    /* add elevator status */
                    rec_buf = this->lora_signal(rec_buf, signal, i);
                }
            }
            if (!rec_buf.isEmpty()) {
                ui->logBrowser->insertPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") +
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
            QMessageBox::information(this, "提示消息", tr("没有数据需要保存！"), QMessageBox::Ok);
            return;
        }
        QString fileName = QFileDialog::getSaveFileName(this, tr("保存为"), tr("未命名.txt"));
        QFile file(fileName);
        /* 如果用户取消了保存则直接退出函数 */
        if (file.fileName().isEmpty()) {
            return;
        }
        /* 如果打开失败则给出提示并退出函数 */
        if (!file.open(QFile::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("保存文件"),
                                 tr("打开文件 %1 失败, 无法保存\n%2").arg(fileName).arg(file.errorString()),
                                 QMessageBox::Ok);
            return;
        }
        /* 写数据到文件 */
        QTextStream out(&file);
        out << ui->logBrowser->toPlainText();
        file.close();
    }
}

/* draw line */

/* lora receive signal quality chart */
void MainWindow::draw_signal_quality_line() {
    QtCharts::QChart *chart = new QtCharts::QChart();
    /* set legend */
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->hide();
    chart->addSeries(LoraSignalQualitySeries);
    // chart->createDefaultAxes();/* set Axis */
    auto axisX = new QtCharts::QValueAxis();
    auto axisY = new QtCharts::QValueAxis();
    // axisX->setRange(1, 100);            /* set axisX range */
    /* set AxisX */
    axisX->setTitleText("time(s)"); /* set axisX title */
    /* set AxisY */
    axisY->setTitleText("quality(dBm)"); /* set axisX title */
    axisY->setRange(-130, -50);          /* set recv signal range*/
    /* add axis to chart */
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    LoraSignalQualitySeries->attachAxis(axisX);
    LoraSignalQualitySeries->attachAxis(axisY);
    /* set chart title */
    chart->setTitle("lora rec signal quality");
    /* show chart */
    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(800, 600);
    chartView->show();
}

/* click button to show signal quality line chart */
void MainWindow::on_signal_quality_button_clicked(bool checked) {
    if (checked) {
        draw_signal_quality_line();
        draw_floor_changed_line();
    }
}

/* floor changed chart */
void MainWindow::draw_floor_changed_line() {
    QtCharts::QChart *chart = new QtCharts::QChart();
    /* set legend */
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->hide();
    chart->addSeries(floorChangedSeries);
    // chart->createDefaultAxes();/* set Axis */
    auto axisX = new QtCharts::QValueAxis();
    auto axisY = new QtCharts::QValueAxis();
    // axisX->setRange(1, 100);            /* set axi sX range */
    /* set AxisX */
    axisX->setTitleText("time(s)"); /* set axisX title */
    /* set AxisY */
    axisY->setTitleText("floor"); /* set axisX title */
    /* add axis to chart */
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    floorChangedSeries->attachAxis(axisX);
    floorChangedSeries->attachAxis(axisY);
    /* set chart title */
    chart->setTitle("floor changed line");
    /* show chart */
    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(800, 600);
    chartView->show();
}
