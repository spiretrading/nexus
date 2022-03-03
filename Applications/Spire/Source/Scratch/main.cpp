#include <QApplication>
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorTableModel.hpp"
#include "Spire/Spire/Intervals.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace boost::posix_time;
using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto source = std::make_shared<LocalOrderImbalanceIndicatorModel>();
  auto model = new OrderImbalanceIndicatorTableModel(
    TimeInterval::closed(from_time_t(0), from_time_t(200)), source);
  application->exec();
}
