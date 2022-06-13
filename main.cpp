#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  /* 解决分辨率不同导致的文件压缩问题 */
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();
}
