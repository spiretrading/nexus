#include <algorithm>
#include <QApplication>
#include <QScreen>
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Blotter/MonitoredTasksModel.hpp"
#include "Spire/BlotterUiTester/BlotterController.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Blotter UI Tester"));
  application.setQuitOnLastWindowClosed(true);
  initialize_resources();
  auto environment = TestEnvironment();
  auto profile = UserProfile("root", true, true, {}, {},
    get_default_additional_tag_database(), {}, {}, {},
    Clients(std::in_place_type<TestClients>, Ref(environment)));
  auto monitors = make_default_task_monitors();
  auto tasks = std::make_shared<MonitoredTasksModel>(monitors);
  auto model = std::make_shared<CompositeBlotterModel>(
    std::make_shared<LocalTextModel>(QObject::tr("New Blotter")),
    std::make_shared<LocalBooleanModel>(false),
    std::make_shared<LocalBooleanModel>(false), tasks, monitors);
  auto controller = BlotterController(tasks, Ref(environment), Ref(profile));
  auto window = BlotterWindow(model);
  window.connect_cancel_tasks_signal(
    std::bind_front(&BlotterController::on_cancel_tasks, &controller));
  window.show();
  controller.show();
  auto screen = QApplication::primaryScreen()->availableGeometry();
  auto gap = scale_height(20);
  window.move(screen.topLeft());
  controller.move(screen.left(), std::min(window.frameGeometry().bottom() + gap,
    screen.bottom() - controller.frameGeometry().height()));
  return application.exec();
}
