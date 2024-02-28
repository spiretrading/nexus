#include <QApplication>
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/LegacyKeyBindings/HotkeyOverride.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("KeyBindings Ui Tester"));
  initialize_resources();
  auto window = KeyBindingsWindow();
  window.show();
  auto hotkey_override = HotkeyOverride();
  application.exec();
}
