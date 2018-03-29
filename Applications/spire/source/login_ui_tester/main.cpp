#include <QApplication>
#include "spire/login_ui_tester/login_ui_tester.hpp"
#include "spire/login/login_window.hpp"
#include "spire/spire/resources.hpp"
#include "spire/ui/window.hpp"
#include "spire/version.hpp"

using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Login UI Tester"));
  initialize_resources();
  login_window login(SPIRE_VERSION);
  login.show();
  login_ui_tester tester(&login);
  login.installEventFilter(&tester);
  tester.setGeometry(login.pos().x(),
    login.pos().y() + login.height() + 100, 0, 0);
  tester.setAttribute(Qt::WA_ShowWithoutActivating);
  tester.show();
  application->exec();
}
