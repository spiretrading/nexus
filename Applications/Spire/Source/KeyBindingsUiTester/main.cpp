#include <QApplication>
#include "Nexus/Definitions/Region.hpp"
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Key Bindings UI Tester"));
  initialize_resources();
  auto bindings = KeyBindings::get_default_key_bindings();
  bindings.set({Qt::Key_Shift, Qt::Key_Escape},
    Nexus::Region(Nexus::Region(Nexus::Region::Global())),
    KeyBindings::CancelAction::ALL);
  bindings.set({Qt::Key_Control, Qt::Key_Escape},
    Nexus::Region(Nexus::Region(Nexus::Region::Global())),
    KeyBindings::CancelAction::FURTHEST_ASK);
  bindings.set({Qt::Key_Alt, Qt::Key_Escape},
    Nexus::Region(Nexus::Region(Nexus::Region::Global())),
    KeyBindings::CancelAction::CLOSEST_ASK);
  auto window = new KeyBindingsWindow(bindings);
  window->show();
  application->exec();
}
