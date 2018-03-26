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
  auto login = new login_window(SPIRE_VERSION);
  spire::window frame(login);
  frame.show();
  login_ui_tester tester(&frame);
  tester.setGeometry(frame.pos().x(),
    frame.pos().y() + frame.height() + 100, 0, 0);
  frame.installEventFilter(&tester);
  tester.setAttribute(Qt::WA_ShowWithoutActivating);
  tester.show();
  application->exec();
}
