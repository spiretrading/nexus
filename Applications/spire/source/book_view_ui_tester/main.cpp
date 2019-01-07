#include <Beam/Threading/TimerThreadPool.hpp>
#include <QApplication>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "spire/security_input/local_security_input_model.hpp"
#include "spire/spire/resources.hpp"
#include "spire/book_view_ui_tester/book_view_test_controller_window.hpp"
#include "spire/book_view/book_view_window.hpp"

using namespace Beam;
using namespace Beam::Threading;
using namespace Nexus;
using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Book View UI Tester"));
  initialize_resources();
  TimerThreadPool timer_thread_pool;
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
  auto window = new BookViewWindow(BookViewProperties(), Ref(model));
  window->show();
  auto tester = new BookViewTestControllerWindow(window, timer_thread_pool);
  tester->show();
  tester->move(window->pos().x() + window->width() + 100,
    tester->pos().y());
  window->activateWindow();
  window->connect_closed_signal([=] { tester->close(); });
  application->exec();
}
