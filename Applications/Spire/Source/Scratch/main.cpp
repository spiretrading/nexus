#include <QApplication>
#include <QTimer>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Charting/LocalChartModel.hpp"
#include "Spire/Charting/ChartingWindow.hpp"
#include "Spire/Charting/ChartView.hpp"
#include "Spire/SecurityInput/LocalSecurityInputModel.hpp"
#include "Spire/Spire/LocalTechnicalsModel.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  LocalSecurityInputModel model;
  model.add(SecurityInfo(
    Security("MSFT", DefaultMarkets::NASDAQ(), DefaultCountries::US()),
    "Microsoft Corp", "Software", 100));
  auto window = new ChartingWindow(Ref(model));
  window->connect_security_change_signal(
    [=] (const auto& security) {
      auto d = date(2020, 5, 5);
      auto candlesticks = std::vector<Candlestick>();
      candlesticks.emplace_back(Scalar(ptime(d, time_duration(0, 59, 0))),
        Scalar(ptime(d, time_duration(1, 0, 0))), Scalar(Money::ZERO),
        Scalar(Money::ONE), Scalar(Money::ONE), Scalar(Money::ZERO));
      candlesticks.emplace_back(Scalar(ptime(d, time_duration(10, 59, 0))),
        Scalar(ptime(d, time_duration(11, 0, 0))), Scalar(Money::ZERO),
        Scalar(Money::ONE), Scalar(Money::ONE), Scalar(Money::ZERO));
      auto chart_model = std::make_shared<LocalChartModel>(
        Scalar::Type::TIMESTAMP, Scalar::Type::MONEY, candlesticks);
      auto technicals_model = std::make_shared<LocalTechnicalsModel>(
        security);
      window->set_models(chart_model, technicals_model);
/*
      window->m_chart->set_region(ChartView::Region{ChartPoint(
        Scalar(ptime(d, time_duration(0, 59, 0))), Scalar(10 * Money::ONE)),
        ChartPoint(Scalar(ptime(d, time_duration(2, 39, 0))),
        Scalar(Money::ZERO))});
*/
      window->m_chart->set_region(ChartView::Region{ChartPoint(
        Scalar(ptime(d, time_duration(10, 50, 0))), Scalar(10 * Money::ONE)),
        ChartPoint(Scalar(ptime(d, time_duration(13, 30, 0))),
        Scalar(Money::ZERO))});
    });
  window->show();
  application->exec();
}
