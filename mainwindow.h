#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QList>
#include <QMessageBox>
#include <QTextStream>
#include <QTimer>
#include <iostream>
#include "lora.h"
#include "transport_crc.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

 public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void draw_signal_quality_line();
    void draw_floor_changed_line();
    void serial_config_disable(bool value);
    void elevator_send_config_enabled(bool value);
    /* lora */
    QString lora_signal(QString str, QString signal, int &i);
    void lora_send_user_data(QString str);
    void lora_init(void);
    void send_cmd_enabled(bool value);
    void query_elevator_status();
    QTimer *query_ele_timer;

 protected:
    /* find free serial */
    void findFreeports();
    /* init serial port */
    bool initSerialPort();
    /* send messsager */
    void sendMsg(const QString &msg);

 public slots:
    void recvMsg();
    void on_clearRxLog_clicked();
    void on_clearTxLog_clicked();
    void on_saveFile_clicked(bool checked);
    void on_signal_quality_button_clicked(bool checked);
    /* lora send button slot function */
    void on_save_send_config_clicked(bool checked);
    void on_circle_query_box_clicked(bool checked);
    void on_preempt_elevator_button_clicked();
    void on_call_elevator_button_clicked();
    void on_query_elevator_status_button_clicked();
    void on_open_door_button_clicked();
    void on_close_door_button_clicked();
    void on_release_elevator_button_clicked();
    void on_cancel_elevator_task_button_clicked();
    void on_reset_elevator_button_clicked();

 private:
    Ui::MainWindow *ui;
    QtCharts::QLineSeries *LoraSignalQualitySeries; /* lora signal quality series */
    QtCharts::QLineSeries *floorChangedSeries;      /* floor changed chart for signal quality */
    QSerialPort *serialPort;
    QString write2fileName;
};

#endif  // MAINWINDOW_H
