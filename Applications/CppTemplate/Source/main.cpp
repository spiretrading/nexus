#include <iostream>
#include <QtWidgets/QApplication>
#include <QMessageBox>
#include <QLabel>
#include <QtPlugin>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSlider>
#include <QSpinBox>
#include <CppTemplate/notepad.hpp>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
int main(int argc, char** argv){
  QApplication app(argc, argv);
  
  Notepad w;
  w.show();


  return app.exec();
}
