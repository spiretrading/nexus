#include "Spire/Login/LoginDialog.hpp"
#include <QApplication>
#include <QtPlugin>

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
