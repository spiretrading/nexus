#include <QApplication>
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/BlotterUiTester/BlotterController.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Blotter UI Tester"));
  application.setQuitOnLastWindowClosed(true);
  initialize_resources();
  auto environment = TestEnvironment();
  auto profile = UserProfile("root", true, true, {}, {},
    get_default_additional_tag_database(), {}, {}, {},
    Clients(std::in_place_type<TestClients>, Ref(environment)));
  auto controller = BlotterController(Ref(environment), Ref(profile));
  controller.show();
  return application.exec();
}
