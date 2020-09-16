#include <QApplication>
#include "Spire/LoginUiTester/LoginUiTester.hpp"
#include "Spire/Login/LoginWindow.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Window.hpp"
#include "Version.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Login UI Tester"));
  initialize_resources();
  LoginWindow login(SPIRE_VERSION);
  login.show();
  LoginUiTester tester(&login);
  login.installEventFilter(&tester);
  tester.setGeometry(login.pos().x(),
    login.pos().y() + login.height() + 100, 0, 0);
  tester.setAttribute(Qt::WA_ShowWithoutActivating);
  tester.show();
  application->exec();
}
