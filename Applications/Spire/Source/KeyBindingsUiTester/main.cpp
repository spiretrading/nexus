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

std::shared_ptr<TickerInfoQueryModel> populate_ticker_query_model() {
  auto ticker_infos = std::vector<TickerInfo>();
  auto add_ticker = [&] (const Ticker& ticker, const std::string& name) {
    if(ticker) {
      ticker_infos.emplace_back(ticker, name, "", 0);
    }
  };
  add_ticker(parse_ticker("MRU.TSX"), "Metro Inc.");
  add_ticker(parse_ticker("MG.TSX"), "Magna International Inc.");
  add_ticker(parse_ticker("MGA.TSX"), "Mega Uranium Ltd.");
  add_ticker(parse_ticker("MGAB.TSX"),
    "Mackenzie Global Fixed Income Alloc ETF");
  add_ticker(parse_ticker("MON.NYSE"), "Monsanto Co.");
  add_ticker(parse_ticker("MFC.TSX"), "Manulife Financial Corporation");
  add_ticker(parse_ticker("MX.TSX"), "Methanex Corporation");
  auto tickers = std::make_shared<LocalQueryModel<TickerInfo>>();
  for(auto& ticker_info : ticker_infos) {
    tickers->add(to_text(ticker_info.m_ticker).toLower(), ticker_info);
    tickers->add(
      QString::fromStdString(ticker_info.m_name).toLower(), ticker_info);
  }
  return tickers;
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
  auto window = KeyBindingsWindow(key_bindings, populate_ticker_query_model(),
    get_default_additional_tag_database());
  window.show();
  auto hotkey_override = HotkeyOverride();
  application.installNativeEventFilter(&hotkey_override);
  application.exec();
}
