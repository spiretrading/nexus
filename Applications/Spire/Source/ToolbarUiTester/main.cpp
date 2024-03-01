#include <QApplication>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/ToolbarUiTester/ToolbarUiTester.hpp"
#include "Version.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Toolbar UI Tester"));
  initialize_resources();
  auto tester = ToolbarUiTester();
  tester.resize(scale(650, 300));
  tester.show();
  application.exec();
}
