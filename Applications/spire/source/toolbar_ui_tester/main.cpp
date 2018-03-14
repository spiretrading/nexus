#include <QApplication>
#include "spire/spire/resources.hpp"
#include "spire/toolbar/toolbar_window.hpp"
#include "spire/ui/window.hpp"

using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Toolbar UI Tester"));
  initialize_resources();
  toolbar_window window;
  spire::window frame(&window);
  frame.show();
  application->exec();
}
