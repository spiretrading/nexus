#include <Beam/Threading/TimerThreadPool.hpp>
#include <QApplication>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "spire/security_input/local_security_input_model.hpp"
#include "spire/spire/resources.hpp"
#include "spire/time_and_sales/time_and_sales_window.hpp"
#include "spire/time_and_sales_ui_tester/time_and_sales_test_controller_window.hpp"

using namespace Beam;
using namespace Beam::Threading;
using namespace Nexus;
using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Time and Sales UI Tester"));
  initialize_resources();
  TimerThreadPool timer_thread_pool;
  local_security_input_model model;
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
  auto test_window = new time_and_sales_window(time_and_sales_properties(),
    model);
  auto tester = new time_and_sales_test_controller_window(test_window,
    timer_thread_pool);
  test_window->show();
  tester->show();
  tester->move(test_window->pos().x() + test_window->width() + 100,
    tester->pos().y());
  test_window->activateWindow();
  test_window->connect_closed_signal([=] { tester->close(); });
  application->exec();
}
