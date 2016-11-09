#include <iostream>
#include <QtWidgets/QApplication>
#include <QMessageBox>
#include <QLabel>
#include <QtPlugin>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSlider> 
#include <QSpinBox>
#include <Spire/LoginDialog.hpp>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)

inline void InitializeResources() {
  Q_INIT_RESOURCE(Resources);
}
int main(int argc, char** argv){
  QApplication app(argc, argv);
  InitializeResources();
  LoginDialog ld;
  ld.show();

  return app.exec();
}
