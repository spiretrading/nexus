#include <QApplication>
#include "spire/version.hpp"
#include "spire/login_ui_tester/login_ui_tester.hpp"
#include "spire/login/login_window.hpp"
#include "spire/spire/resources.hpp"

using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Login UI Tester"));
  initialize_resources();
  login_window window(SPIRE_VERSION);
  window.show();
  login_ui_tester tester(&window);
  tester.show();
  application->exec();
}
