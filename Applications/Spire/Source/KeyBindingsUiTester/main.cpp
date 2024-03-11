#include <QApplication>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QVBoxLayout>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Spire/KeyBindings/HotkeyOverride.hpp"
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace Nexus;
using namespace Spire;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("KeyBindings Ui Tester"));
  application.setQuitOnLastWindowClosed(true);
  initialize_resources();
  auto key_bindings =
    std::make_shared<KeyBindingsModel>(GetDefaultMarketDatabase());
  auto window = KeyBindingsWindow(key_bindings);
  window.show();
  auto hotkey_override = HotkeyOverride();
  application.exec();
}
