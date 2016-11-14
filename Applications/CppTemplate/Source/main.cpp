#include <iostream>
#include <QtWidgets/QApplication>
#include <QMessageBox>
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
int main(int argc, char** argv){
  QApplication app(argc, argv);
  QMessageBox msgBox;
  msgBox.setText("HELLO WORLD");
  msgBox.exec();
  return app.exec();
}
