#include <QApplication>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/TimeAndSales/LocalTimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Nexus;
using namespace Spire;

std::shared_ptr<ComboBox::QueryModel> populate_security_query_model() {
  auto security_infos = std::vector<SecurityInfo>();
  security_infos.emplace_back(*ParseWildCardSecurity("MRU.TSX",
    GetDefaultMarketDatabase(), GetDefaultCountryDatabase()),
    "Metro Inc.", "", 0);
  security_infos.emplace_back(*ParseWildCardSecurity("MG.TSX",
    GetDefaultMarketDatabase(), GetDefaultCountryDatabase()),
    "Magna International Inc.", "", 0);
  security_infos.emplace_back(*ParseWildCardSecurity("MGA.TSX",
    GetDefaultMarketDatabase(), GetDefaultCountryDatabase()),
    "Mega Uranium Ltd.", "", 0);
  security_infos.emplace_back(*ParseWildCardSecurity("MGAB.TSX",
    GetDefaultMarketDatabase(), GetDefaultCountryDatabase()),
    "Mackenzie Global Fixed Income Alloc ETF", "", 0);
  security_infos.emplace_back(*ParseWildCardSecurity("MON.NYSE",
    GetDefaultMarketDatabase(), GetDefaultCountryDatabase()),
    "Monsanto Co.", "", 0);
  security_infos.emplace_back(*ParseWildCardSecurity("MFC.TSX",
    GetDefaultMarketDatabase(), GetDefaultCountryDatabase()),
    "Manulife Financial Corporation", "", 0);
  security_infos.emplace_back(*ParseWildCardSecurity("MX.TSX",
    GetDefaultMarketDatabase(), GetDefaultCountryDatabase()),
    "Methanex Corporation", "", 0);
  auto model = std::make_shared<LocalComboBoxQueryModel>();
  for(auto security_info : security_infos) {
    model->add(
      displayText(security_info.m_security).toLower(), security_info);
    model->add(
      QString::fromStdString(security_info.m_name).toLower(), security_info);
  }
  return model;
}

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("TimeAndSales Ui Tester"));
  initialize_resources();
  TimeAndSalesWindow window(populate_security_query_model(),
    std::make_shared<TimeAndSalesWindowProperties>());
  window.get_security()->connect_update_signal([&] (const auto& security) {
    auto time_and_sales = std::make_shared<LocalTimeAndSalesModel>(security);
    time_and_sales->set_query_duration(boost::posix_time::seconds(5));
    time_and_sales->set_price(Money(200));
    time_and_sales->set_period(boost::posix_time::seconds(1));
    time_and_sales->set_bbo_indicator(BboIndicator::AT_BID);
    window.set_time_and_sales_model(std::move(time_and_sales));
    });
  window.show();
  application->exec();
}
