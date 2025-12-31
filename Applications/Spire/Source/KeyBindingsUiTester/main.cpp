#include <QApplication>
#include "Spire/KeyBindings/HotkeyOverride.hpp"
#include "Spire/KeyBindings/KeyBindingsProfile.hpp"
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

std::shared_ptr<SecurityInfoQueryModel> populate_security_query_model() {
  auto security_infos = std::vector<SecurityInfo>();
  security_infos.emplace_back(parse_security("MRU.TSX"), "Metro Inc.", "", 0);
  security_infos.emplace_back(
    parse_security("MG.TSX"), "Magna International Inc.", "", 0);
  security_infos.emplace_back(
    parse_security("MGA.TSX"), "Mega Uranium Ltd.", "", 0);
  security_infos.emplace_back(parse_security("MGAB.TSX"),
    "Mackenzie Global Fixed Income Alloc ETF", "", 0);
  security_infos.emplace_back(
    parse_security("MON.NYSE"), "Monsanto Co.", "", 0);
  security_infos.emplace_back(
    parse_security("MFC.TSX"), "Manulife Financial Corporation", "", 0);
  security_infos.emplace_back(
    parse_security("MX.TSX"), "Methanex Corporation", "", 0);
  auto securities = std::make_shared<LocalQueryModel<SecurityInfo>>();
  for(auto& security_info : security_infos) {
    securities->add(to_text(security_info.m_security).toLower(), security_info);
    securities->add(
      QString::fromStdString(security_info.m_name).toLower(), security_info);
  }
  return securities;
}

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("KeyBindings Ui Tester"));
  application.setQuitOnLastWindowClosed(true);
  initialize_resources();
  auto key_bindings = std::make_shared<KeyBindingsModel>();
  auto tasks = make_default_order_task_nodes();
  for(auto& task : tasks) {
    key_bindings->get_order_task_arguments()->push(
      to_order_task_arguments(*task));
  }
  auto window = KeyBindingsWindow(key_bindings, populate_security_query_model(),
    get_default_additional_tag_database());
  window.show();
  auto hotkey_override = HotkeyOverride();
  application.installNativeEventFilter(&hotkey_override);
  application.exec();
}
