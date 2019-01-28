#include <QApplication>
#include <random>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/TechnicalAnalysis/Candlestick.hpp"
#include "spire/charting/local_chart_model.hpp"
#include "spire/charting/charting_window.hpp"
#include "spire/security_input/local_security_input_model.hpp"
#include "spire/spire/resources.hpp"
#include "spire/version.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Charting Window Tester"));
  initialize_resources();
  LocalSecurityInputModel model;
  model.add(SecurityInfo(
    Security("MSFT", DefaultMarkets::NASDAQ(), DefaultCountries::US()),
    "Microsoft Corp", "Software"));
  model.add(SecurityInfo(
    Security("MG", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Magna International Inc.", "Automotive, probably"));
  model.add(SecurityInfo(
    Security("MFC", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Manulife Financial Corporation", "Finance"));
  model.add(SecurityInfo(
    Security("MX", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Methanex Corporation", ""));
  model.add(SecurityInfo(
    Security("MRU", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Metro Inc.", ""));
  model.add(SecurityInfo(
    Security("MON", DefaultMarkets::NYSE(), DefaultCountries::US()),
    "Monsanto Co.", ""));
  model.add(SecurityInfo(
    Security("MS", DefaultMarkets::NYSE(), DefaultCountries::US()),
    "Morgan Stanley", "Finance"));
  auto window = new ChartingWindow(Ref(model));
  auto candlesticks = std::vector<Candlestick>();
  auto rand = std::default_random_engine(std::random_device()());
  auto time = boost::posix_time::second_clock::local_time();
  for(auto i = 0; i < 100; ++i) {
    candlesticks.push_back(Candlestick(ChartValue(time),
      ChartValue(time - boost::posix_time::minutes(1)),
      ChartValue(Money(rand() % 100 * Money::FromValue("0.01").get())),
      ChartValue(Money(rand() % 100 * Money::FromValue("0.01").get())),
      ChartValue(Money(rand() % 100 * Money::FromValue("0.01").get())),
      ChartValue(Money(rand() % 100 * Money::FromValue("0.01").get()))));
      time -= boost::posix_time::minutes(1);
  }
  auto chart_model = std::make_shared<LocalChartModel>(
    ChartValue::Type::TIMESTAMP, ChartValue::Type::MONEY, candlesticks);
  window->set_model(chart_model);
  window->show();
  application->exec();
}
