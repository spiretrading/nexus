#include <random>
#include <QApplication>
#include <QTimer>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/TechnicalAnalysis/Candlestick.hpp"
#include "Spire/Charting/LocalChartModel.hpp"
#include "Spire/Charting/ChartingWindow.hpp"
#include "Spire/SecurityInput/LocalSecurityInputModel.hpp"
#include "Spire/Spire/LocalTechnicalsModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/RecentColors.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Charting Window Tester"));
  initialize_resources();
  auto window = new ChartingWindow(
    Ref(LocalSecurityInputModel::get_test_model()));
  auto test_timer = QTimer();
  window->connect_security_change_signal(
    [=, &test_timer] (const auto& security) {
      test_timer.stop();
      window->setWindowTitle(CustomVariantItemDelegate().displayText(
        QVariant::fromValue(security), QLocale()) + QObject::tr(" - Chart"));
      auto candlesticks = std::vector<Candlestick>();
      auto rand = std::default_random_engine(std::random_device()());
      auto time = boost::posix_time::second_clock::local_time();
      for(auto i = 0; i < 100; ++i) {
        auto open = Scalar(Money((rand() % 40 + 40) *
          Money::FromValue("0.01").get()));
        auto close = Scalar(Money((rand() % 40 + 40) *
          Money::FromValue("0.01").get()));
        auto [high, low] = [&] {
          if(open > close) {
            return std::make_tuple(Scalar(Money((rand() % 40) *
              Money::FromValue("0.01").get())) + open, close - Scalar(Money(
              (rand() % 40) * Money::FromValue("0.01").get())));
          }
          return std::make_tuple(Scalar(Money((rand() % 40) *
            Money::FromValue("0.01").get())) + close, open - Scalar(Money(
            (rand() % 40) * Money::FromValue("0.01").get())));
        }();
        candlesticks.push_back(Candlestick(
          Scalar(time - boost::posix_time::minutes(1)), Scalar(time), open,
          close, high, low));
        if(rand() % 5 == 1) {
          time -= boost::posix_time::minutes(rand() % 25 + 1);
        } else {
          time -= boost::posix_time::minutes(1);
        }
      }
      auto chart_model = std::make_shared<LocalChartModel>(
        Scalar::Type::TIMESTAMP, Scalar::Type::MONEY, candlesticks);
      auto technicals_model = std::make_shared<LocalTechnicalsModel>(
        security);
      test_timer.start(1500);
      QObject::connect(&test_timer, &QTimer::timeout, [=] {
        auto rand = std::default_random_engine(std::random_device()())() % 100;
        technicals_model->update(TimeAndSale(boost::posix_time::ptime(),
          Money(rand * Money::ONE), 100, TimeAndSale::Condition(), "null"));
      });
      window->set_models(chart_model, technicals_model);
    });
  window->show();
  application->exec();
}
