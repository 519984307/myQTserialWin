#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "lora.h"
#include "transport_crc.h"
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QList>
#include <QMessageBox>
#include <QTextStream>
#include <iostream>

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
  /* lora */
  QString lora_signal(QString str, QString signal, int &i);
  void lora_init(void);

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

private:
  Ui::MainWindow *ui;
  QtCharts::QLineSeries
      *LoraSignalQualitySeries; /* lora signal quality series */
  QtCharts::QLineSeries
      *floorChangedSeries; /* floor changed chart for signal quality */
  QSerialPort *serialPort;
  QString write2fileName;
};

#endif // MAINWINDOW_H
