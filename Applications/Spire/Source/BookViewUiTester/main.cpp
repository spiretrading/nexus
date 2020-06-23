#include <Beam/Threading/TimerThreadPool.hpp>
#include <QApplication>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Spire/SecurityInput/LocalSecurityInputModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/BookViewUiTester/BookViewTestControllerWindow.hpp"
#include "Spire/BookView/BookViewWindow.hpp"

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
    "Microsoft Corp", "Software", 100));
  model.add(SecurityInfo(
    Security("MG", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Magna International Inc.", "Automotive", 100));
  model.add(SecurityInfo(
    Security("MFC", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Manulife Financial Corporation", "Finance", 100));
  model.add(SecurityInfo(
    Security("MX", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Methanex Corporation", "", 100));
  model.add(SecurityInfo(
    Security("MRU", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Metro Inc.", "", 100));
  model.add(SecurityInfo(
    Security("MON", DefaultMarkets::NYSE(), DefaultCountries::US()),
    "Monsanto Co.", "", 100));
  model.add(SecurityInfo(
    Security("MS", DefaultMarkets::NYSE(), DefaultCountries::US()),
    "Morgan Stanley", "Finance", 100));
  auto window = new BookViewWindow(BookViewProperties(),
    RecentColors::get_default_colors(), Ref(model));
  window->show();
  auto tester = new BookViewTestControllerWindow(window, timer_thread_pool);
  tester->show();
  tester->move(window->pos().x() + window->width() + 100,
    tester->pos().y());
  window->activateWindow();
  application->exec();
}
