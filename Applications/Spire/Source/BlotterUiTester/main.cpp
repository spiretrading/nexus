#include <QApplication>
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Window.hpp"
#include "Version.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Blotter UI Tester"));
  initialize_resources();
  auto blotter = BlotterWindow();
  blotter.show();
  application.exec();
}