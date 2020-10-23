#include <QApplication>
#include "Nexus/Definitions/Region.hpp"
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/SecurityInput/LocalSecurityInputModel.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Key Bindings UI Tester"));
  initialize_resources();
  auto bindings = KeyBindings::get_default_key_bindings();
  auto region = Region(Security("MSFT", DefaultMarkets::NASDAQ(),
    DefaultCountries::US()));
  auto action1 = KeyBindings::OrderAction("Name 1",
    {{40, OrderType(OrderType::LIMIT)}, {54, Side(Side::BID)},
     {59, TimeInForce(TimeInForce::Type::GTC)}, {38, Quantity(456)}});
  bindings.set(Qt::Key_F1, region, action1);
  auto action2 = KeyBindings::OrderAction("Name 2",
    {{40, OrderType(OrderType::LIMIT)}, {54, Side(Side::BID)},
     {59, TimeInForce(TimeInForce::Type::GTC)}, {38, Quantity(789)}});
  bindings.set(Qt::Key_F2, region, action2);
  bindings.set({Qt::Key_Shift, Qt::Key_Escape}, Region::Global(),
    KeyBindings::CancelAction::ALL);
  bindings.set({Qt::Key_Control, Qt::Key_Escape}, Region::Global(),
    KeyBindings::CancelAction::FURTHEST_ASK);
  bindings.set({Qt::Key_Alt, Qt::Key_Escape}, Region::Global(),
    KeyBindings::CancelAction::CLOSEST_ASK);
  auto window = new KeyBindingsWindow(bindings,
    Ref(LocalSecurityInputModel::get_test_model()));
  window->show();
  application->exec();
}
