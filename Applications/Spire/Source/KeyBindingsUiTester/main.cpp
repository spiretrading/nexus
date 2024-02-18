#include <QApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextEdit>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Spire/KeyBindings/CompositeKeyBindingsModel.hpp"
#include "Spire/KeyBindings/KeyBindingsWindow.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/layouts.hpp"

using namespace Nexus;
using namespace Spire;

auto populate_region_query_model() {
  auto securities = std::vector<std::pair<std::string, std::string>>{
    {"MSFT.NSDQ", "Microsoft Corporation"},
    {"MG.TSX", "Magna International Inc."},
    {"MRU.TSX", "Metro Inc."},
    {"MFC.TSX", "Manulife Financial Corporation"},
    {"MX.TSX", "Methanex Corporation"},
    {"TSO.ASX", "Tesoro Resources Limited"}};
  auto markets = std::vector<MarketCode>{DefaultMarkets::NSEX(),
    DefaultMarkets::ISE(), DefaultMarkets::CSE(), DefaultMarkets::TSX(),
    DefaultMarkets::TSXV()};
  auto countries = std::vector<CountryCode>{DefaultCountries::US(),
    DefaultCountries::CA(), DefaultCountries::AU()};
  auto model = std::make_shared<LocalComboBoxQueryModel>();
  for(auto& security_info : securities) {
    auto security = *ParseWildCardSecurity(security_info.first,
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
    auto region = Region(security);
    region.SetName(security_info.second);
    model->add(to_text(security).toLower(), region);
    model->add(QString::fromStdString(region.GetName()).toLower(), region);
  }
  for(auto& market_code : markets) {
    auto market = GetDefaultMarketDatabase().FromCode(market_code);
    auto region = Region(market);
    region.SetName(market.m_description);
    model->add(to_text(MarketToken(market.m_code)).toLower(), region);
    model->add(QString::fromStdString(region.GetName()).toLower(), region);
  }
  for(auto& country : countries) {
    auto region = Region(country);
    region.SetName(
      GetDefaultCountryDatabase().FromCode(country).m_name);
    model->add(to_text(country).toLower(), region);
    model->add(QString::fromStdString(region.GetName()).toLower(), region);
  }
  return model;
}

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("KeyBindings Ui Tester"));
  initialize_resources();
  auto order_tasks = std::make_shared<ArrayListModel<OrderTask>>();
  order_tasks->push({"Test1",
    Region(*ParseWildCardSecurity(
      "MSFT.NSDQ", GetDefaultMarketDatabase(), GetDefaultCountryDatabase())),
    "NASDAQ", OrderType::MARKET, Side::ASK, Quantity(1),
    TimeInForce(TimeInForce::Type::DAY), QKeySequence("Ctrl+F4")});
  order_tasks->push({"Test2",
    Region(GetDefaultMarketDatabase().FromCode(DefaultMarkets::TSX())),
    "TSX", OrderType::STOP, Side::ASK, Quantity(10),
    TimeInForce(TimeInForce::Type::DAY), QKeySequence("Ctrl+Alt+S")});
  order_tasks->push({"Test3",
    Region(DefaultCountries::US()), "NYSE",
    OrderType::MARKET, Side::BID, Quantity(20),
    TimeInForce(TimeInForce::Type::IOC), QKeySequence("F3")});
  auto default_order_tasks = std::make_shared<ArrayListModel<OrderTask>>();
  default_order_tasks->push({"TSX DRK Limit Bid",
    Region(GetDefaultMarketDatabase().FromCode(DefaultMarkets::TSX())), "LYNX",
    OrderType::LIMIT, Side::ASK, Quantity(1500),
    TimeInForce(TimeInForce::Type::DAY), QKeySequence("F3")});
  default_order_tasks->push({"TSX Limit Bid",
    Region(GetDefaultMarketDatabase().FromCode(DefaultMarkets::TSXV())), "CX2",
    OrderType::LIMIT, Side::ASK, Quantity(1000),
    TimeInForce(TimeInForce::Type::DAY), QKeySequence("Shift+F1")});
  auto window = KeyBindingsWindow(
    std::make_shared<CompositeKeyBindingsModel>(std::move(order_tasks),
      std::move(default_order_tasks), populate_region_query_model(),
      GetDefaultDestinationDatabase(), GetDefaultMarketDatabase()));
  window.show();
  auto output_widget = QWidget();
  auto output = new QTextEdit();
  output->setReadOnly(true);
  enclose(output_widget, *output);
  output_widget.show();
  output_widget.resize(scale_width(300), window.height());
  output_widget.move(window.pos().x() + window.width() + 1, window.pos().y());
  window.connect_submit_signal([&] (const auto& order_tasks_submission) {
    if(order_tasks_submission) {
      auto order_tasks_json = QJsonObject();
      for(auto i = 0; i < order_tasks_submission->get_size(); ++i) {
        auto& order_task = order_tasks_submission->get(i);
        auto json = QJsonObject();
        json["Name"] = order_task.m_name;
        auto region_json = QJsonObject();
        auto country_json = QJsonArray();
        for(auto& country : order_task.m_region.GetCountries()) {
          country_json.append(to_text(country));
        }
        region_json["Countries"] = country_json;
        auto market_json = QJsonArray();
        for(auto& market : order_task.m_region.GetMarkets()) {
          market_json.append(to_text(MarketToken(market)));
        }
        region_json["Markets"] = market_json;
        auto security_json = QJsonArray();
        for(auto& security : order_task.m_region.GetSecurities()) {
          security_json.append(to_text(security));
        }
        region_json["Securities"] = security_json;
        json["Region"] = region_json;
        json["Destination"] = to_text(order_task.m_destination);
        json["Order Type"] = to_text(order_task.m_order_type);
        json["Side"] = to_text(order_task.m_side);
        if(order_task.m_quantity) {
          json["Quantity"] = to_text(*order_task.m_quantity);
        }
        json["Time in Force"] = to_text(order_task.m_time_in_force);
        json["Key"] = order_task.m_key.toString();
        order_tasks_json[QString("Order Task %1").arg(i + 1)] = json;
      }
      auto doc = QJsonDocument(order_tasks_json);
      output->append("Submission:");
      output->append(doc.toJson(QJsonDocument::Indented));
    }
  });
  application.exec();
}
