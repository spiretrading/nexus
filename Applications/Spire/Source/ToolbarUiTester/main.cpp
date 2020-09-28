#include <QApplication>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Toolbar/RecentlyClosedModel.hpp"
#include "Spire/Toolbar/ToolbarWindow.hpp"
#include "Spire/ToolbarUiTester/ToolbarUiTester.hpp"
#include "Spire/Ui/Window.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Toolbar UI Tester"));
  initialize_resources();
  RecentlyClosedModel model;
  auto account = DirectoryEntry::MakeAccount(1, "demo_account");
  ToolbarWindow tw(Ref(model), account);
  tw.show();
  ToolbarUiTester tester(&tw, model);
  tester.show();
  application->exec();
}
