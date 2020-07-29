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
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Key Bindings UI Tester"));
  initialize_resources();
  auto bindings = KeyBindings::get_default_key_bindings();
  auto region = Region(Security("MSFT", DefaultMarkets::NASDAQ(),
    DefaultCountries::US()));
  auto action1 = KeyBindings::OrderAction{"Name 1",
    {{40, OrderType(OrderType::LIMIT)}, {54, Side(Side::BID)},
     {59, TimeInForce(TimeInForce::Type::GTC)}, {38, Quantity(456)}}};
  bindings.set(Qt::Key_F1, region, action1);
  auto action2 = KeyBindings::OrderAction{"Name 2",
    {{40, OrderType(OrderType::LIMIT)}, {54, Side(Side::BID)},
     {59, TimeInForce(TimeInForce::Type::GTC)}, {38, Quantity(789)}}};
  bindings.set(Qt::Key_F2, region, action2);
  bindings.set({Qt::Key_Shift, Qt::Key_Escape}, Region::Global(),
    KeyBindings::CancelAction::ALL);
  bindings.set({Qt::Key_Control, Qt::Key_Escape}, Region::Global(),
    KeyBindings::CancelAction::FURTHEST_ASK);
  bindings.set({Qt::Key_Alt, Qt::Key_Escape}, Region::Global(),
    KeyBindings::CancelAction::CLOSEST_ASK);
  auto input_model = LocalSecurityInputModel();
  input_model.add(SecurityInfo(
    Security("MSFT", DefaultMarkets::NASDAQ(), DefaultCountries::US()),
    "Microsoft Corp", "Software", Quantity(100)));
  input_model.add(SecurityInfo(
    Security("MG", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Magna International Inc.", "Automotive, probably", Quantity(100)));
  input_model.add(SecurityInfo(
    Security("MFC", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Manulife Financial Corporation", "Finance", Quantity(100)));
  input_model.add(SecurityInfo(
    Security("MX", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Methanex Corporation", "", Quantity(100)));
  input_model.add(SecurityInfo(
    Security("MRU", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Metro Inc.", "", Quantity(100)));
  input_model.add(SecurityInfo(
    Security("MON", DefaultMarkets::NYSE(), DefaultCountries::US()),
    "Monsanto Co.", "", Quantity(100)));
  input_model.add(SecurityInfo(
    Security("MS", DefaultMarkets::NYSE(), DefaultCountries::US()),
    "Morgan Stanley", "Finance", Quantity(100)));
  auto window = new KeyBindingsWindow(bindings, Ref(input_model));
  window->show();
  application->exec();
}
