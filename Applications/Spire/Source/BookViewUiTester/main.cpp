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
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Book View UI Tester"));
  initialize_resources();
  auto window = new BookViewWindow(BookViewProperties(),
    Ref(LocalSecurityInputModel::get_test_model()));
  window->show();
  auto tester = new BookViewTestControllerWindow(window);
  tester->show();
  tester->move(window->pos().x() + window->width() + 100, tester->pos().y());
  window->activateWindow();
  application->exec();
}
