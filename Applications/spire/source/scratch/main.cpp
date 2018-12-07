#include <QApplication>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "spire/charting/chart_point.hpp"
#include "spire/charting/chart_value.hpp"
#include "spire/charting/chart_view.hpp"
#include "spire/spire/resources.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = new ChartView(ChartValue::Type::DURATION, ChartValue::Type::MONEY);
  auto current_time = boost::posix_time::second_clock::local_time();
  auto bottom_right = ChartPoint(
    ChartValue(current_time),
    ChartValue(Nexus::Money(10)));
  auto top_left = ChartPoint(
    ChartValue(current_time - boost::posix_time::hours(1)),
    ChartValue(Nexus::Money(10.10)));
  w->set_region(top_left, bottom_right);
  w->resize(500, 500);
  w->show();
  application->exec();
}
