#include <QApplication>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Spire/SecurityInput/LocalSecurityInputModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/TimeAndSalesUiTester/TimeAndSalesTestControllerWindow.hpp"

using namespace Beam;
using namespace Beam::Threading;
using namespace Nexus;
using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Time and Sales UI Tester"));
  initialize_resources();
  auto test_window = new TimeAndSalesWindow(TimeAndSalesProperties(),
    Ref(LocalSecurityInputModel::get_test_model()));
  auto tester = new TimeAndSalesTestControllerWindow(test_window);
  test_window->show();
  tester->show();
  tester->move(test_window->pos().x() + test_window->width() + 100,
    tester->pos().y());
  test_window->activateWindow();
  application->exec();
}
