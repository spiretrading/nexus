#include <QApplication>
#include "Nexus/Definitions/Region.hpp"
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace Nexus;
using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Key Bindings UI Tester"));
  initialize_resources();
  auto bindings = KeyBindings::get_default_key_bindings();
  auto region = Region(Security("ASDF", 0));
  auto action1 = KeyBindings::OrderAction{"Name 1", OrderType::LIMIT,
    Side::BID, TimeInForce(TimeInForce::Type::GTC), 456, {}};
  bindings.set(Qt::Key_F1, region, action1);
  auto action2 = KeyBindings::OrderAction{"Name 2", OrderType::LIMIT,
    Side::BID, TimeInForce(TimeInForce::Type::DAY), 789, {}};
  bindings.set(Qt::Key_F2, Region(Region::GlobalTag()), action2);
  bindings.set({Qt::Key_Shift, Qt::Key_Escape},
    Region(Region(Region::GlobalTag{})), KeyBindings::CancelAction::ALL);
  bindings.set({Qt::Key_Control, Qt::Key_Escape},
    Region(Region(Region::GlobalTag{})),
    KeyBindings::CancelAction::FURTHEST_ASK);
  bindings.set({Qt::Key_Alt, Qt::Key_Escape},
    Region(Region(Region::GlobalTag{})),
    KeyBindings::CancelAction::CLOSEST_ASK);
  auto window = new KeyBindingsWindow(bindings);
  window->show();
  application->exec();
}
