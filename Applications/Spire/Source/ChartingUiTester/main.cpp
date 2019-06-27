#include <random>
#include <QApplication>
#include <QTimer>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/TechnicalAnalysis/Candlestick.hpp"
#include "Spire/Charting/CachedChartModel.hpp"
#include "Spire/Charting/LocalChartModel.hpp"
#include "Spire/Charting/ChartingWindow.hpp"
#include "Spire/SecurityInput/LocalSecurityInputModel.hpp"
#include "Spire/Spire/LocalTechnicalsModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Version.hpp"

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
  auto test_timer = QTimer();
  window->connect_security_change_signal(
    [=, &test_timer] (const auto& security) {
      test_timer.stop();
      window->setWindowTitle(CustomVariantItemDelegate().displayText(
        QVariant::fromValue(security), QLocale()) + QObject::tr(" - Chart"));
      auto candlesticks = std::vector<Candlestick>();
      auto rand = std::default_random_engine(std::random_device()());
      auto time = 100 * Money::ONE;//boost::posix_time::second_clock::local_time();
      for(auto i = 0; i < 10; ++i) {
        auto open = ChartValue(Money((rand() % 40 + 40) *
          Money::FromValue("0.01").get()));
        auto close = ChartValue(Money((rand() % 40 + 40) *
          Money::FromValue("0.01").get()));
        auto [high, low] = [&] {
          if(open > close) {
            return std::make_tuple(ChartValue(Money((rand() % 40) *
              Money::FromValue("0.01").get())) + open, close - ChartValue(Money(
              (rand() % 40) * Money::FromValue("0.01").get())));
          }
          return std::make_tuple(ChartValue(Money((rand() % 40) *
            Money::FromValue("0.01").get())) + close, open - ChartValue(Money(
            (rand() % 40) * Money::FromValue("0.01").get())));
        }();
        candlesticks.push_back(Candlestick(
          ChartValue(time - Money::ONE/*time - boost::posix_time::minutes(1)*/), ChartValue(time),
            open, close, high, low));
        if(rand() % 2 == 1) {
          //time -= boost::posix_time::minutes(rand() % 25);
          time -= (rand() % 25) * Money::ONE;
        } else {
          //time -= boost::posix_time::minutes(1);
          time -= Money::ONE;
        }
      }
      auto chart_model = new LocalChartModel(
        ChartValue::Type::MONEY, ChartValue::Type::MONEY, candlesticks);
      auto cached_model = std::make_shared<CachedChartModel>(*chart_model);
      auto technicals_model = std::make_shared<LocalTechnicalsModel>(Security());
      test_timer.start(1500);
      QObject::connect(&test_timer, &QTimer::timeout, [=] {
        auto rand = std::default_random_engine(std::random_device()())() % 100;
        technicals_model->update(TimeAndSale(boost::posix_time::ptime(),
          Money(rand * Money::ONE), 100, TimeAndSale::Condition(), "null"));
      });
      window->set_models(std::move(cached_model), technicals_model);
    });
  window->show();
  application->exec();
}
