#include <iostream>
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QtPlugin>
#include <QSlider>
#include <QSpinBox>
#include <Spire/LoginDialog.hpp>
#include <Qt>

using namespace Spire;

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
