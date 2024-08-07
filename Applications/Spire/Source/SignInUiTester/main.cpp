#include <QApplication>
#include "Spire/SignInUiTester/SignInUiTester.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Version.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("SignIn UI Tester"));
  initialize_resources();
  auto tester = SignInUiTester();
  tester.setAttribute(Qt::WA_ShowWithoutActivating);
  tester.show();
  application.exec();
}
