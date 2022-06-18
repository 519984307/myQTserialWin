#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

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

private:
  Ui::MainWindow *ui;
  QSerialPort *serialPort;
  QString write2fileName;
};
#endif // MAINWINDOW_H
