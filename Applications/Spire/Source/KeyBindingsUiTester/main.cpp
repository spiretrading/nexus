#include <QApplication>
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Key Bindings UI Tester"));
  initialize_resources();
  auto bindings = KeyBindings::get_default_key_bindings();
  auto window = new KeyBindingsWindow(bindings);
  window->show();
  application->exec();
}
