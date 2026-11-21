#include <algorithm>
#include <Beam/Queues/Queue.hpp>
#include <QApplication>
#include <QScreen>
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Blotter/ControllerPortfolioModel.hpp"
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
  auto orders = std::make_shared<Queue<std::shared_ptr<Order>>>();
  environment.monitor_order_submissions(orders);
  auto portfolio = std::make_shared<ControllerPortfolioModel>(
    std::make_shared<ControllerPortfolioModel::Controller>(
      Portfolio(TrueAverageBookkeeper()),
      profile.GetClients().get_market_data_client(), orders));
  auto model = std::make_shared<CompositeBlotterModel>(
    std::make_shared<LocalTextModel>(QObject::tr("New Blotter")),
    std::make_shared<LocalBooleanModel>(false),
    std::make_shared<LocalBooleanModel>(false), tasks, monitors, portfolio);
  auto controller = BlotterController(tasks, Ref(environment), Ref(profile));
  auto window = BlotterWindow(model,
    profile.GetKeyBindings()->get_cancel_key_bindings(),
    std::make_shared<PositionKeyBindingsModel>());
  window.connect_run_tasks_signal(
    std::bind_front(&BlotterController::on_run_tasks, &controller));
  window.connect_cancel_tasks_signal(
    std::bind_front(&BlotterController::on_cancel_tasks, &controller));
  window.connect_cancel_orders_signal(
    std::bind_front(&BlotterController::on_cancel_orders, &controller));
  window.connect_cancel_task_operation_signal(std::bind_front(
    &BlotterController::on_cancel_task_operation, &controller));
  window.connect_cancel_order_operation_signal(std::bind_front(
    &BlotterController::on_cancel_order_operation, &controller));
  window.connect_flatten_positions_signal(
    std::bind_front(&BlotterController::on_flatten_positions, &controller));
  window.show();
  controller.show();
  auto screen = QApplication::primaryScreen()->availableGeometry();
  auto gap = scale_height(20);
  window.move(screen.topLeft());
  controller.move(screen.left(), std::min(window.frameGeometry().bottom() + gap,
    screen.bottom() - controller.frameGeometry().height()));
  return application.exec();
}
