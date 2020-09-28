#include <QApplication>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Spire/SecurityInput/LocalSecurityInputModel.hpp"
#include "Spire/SecurityInput/SecurityInputDialog.hpp"
#include "Spire/SecurityInputUiTester/SecurityInputTester.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace Nexus;
using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Security Input UI Tester"));
  initialize_resources();
  SecurityInputTester tester;
  tester.show();
  application->exec();
}
