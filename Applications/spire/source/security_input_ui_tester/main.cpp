#include <QApplication>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "spire/security_input/local_security_input_model.hpp"
#include "spire/security_input/security_input_dialog.hpp"
#include "spire/security_input_ui_tester/security_input_tester.hpp"
#include "spire/spire/resources.hpp"

using namespace Nexus;
using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Security Input UI Tester"));
  initialize_resources();
  SecurityInputTester tester;
  tester.show();
  application->exec();
}
