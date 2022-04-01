#include <QApplication>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Blotter/CompositeBlotterStatusModel.hpp"
#include "Spire/Blotter/LocalBlotterPositionsModel.hpp"
#include "Spire/Blotter/LocalBlotterProfitAndLossModel.hpp"
#include "Spire/Canvas/Task.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/Window.hpp"
#include "Version.hpp"

using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

namespace {
  struct StatusBarController : QWidget {
    QDoubleSpinBox* m_unrealized_profit_and_loss_box;
    std::shared_ptr<MoneyModel> m_unrealized_profit_and_loss;
    QDoubleSpinBox* m_realized_profit_and_loss_box;
    std::shared_ptr<MoneyModel> m_realized_profit_and_loss;
    QDoubleSpinBox* m_fees_box;
    std::shared_ptr<MoneyModel> m_fees;
    QDoubleSpinBox* m_cost_basis_box;
    std::shared_ptr<MoneyModel> m_cost_basis;
    QDoubleSpinBox* m_buying_power_box;
    std::shared_ptr<MoneyModel> m_buying_power;
    QDoubleSpinBox* m_net_loss_box;
    std::shared_ptr<MoneyModel> m_net_loss;
    std::shared_ptr<MoneyModel> m_total_profit_and_loss;

    StatusBarController() {
      auto layout = new QGridLayout(this);
      add_money_input(*layout, "Unrealized P/L", 0,
        m_unrealized_profit_and_loss_box, m_unrealized_profit_and_loss);
      add_money_input(*layout, "Realized P/L", 1,
        m_realized_profit_and_loss_box, m_realized_profit_and_loss);
      add_money_input(*layout, "Fees", 2, m_fees_box, m_fees);
      add_money_input(*layout, "Cost Basis", 3, m_cost_basis_box, m_cost_basis);
      add_money_input(
        *layout, "Buying Power", 4, m_buying_power_box, m_buying_power);
      add_money_input(*layout, "Net Loss", 5, m_net_loss_box, m_net_loss);
      m_total_profit_and_loss = std::make_shared<LocalMoneyModel>();
      m_unrealized_profit_and_loss->connect_update_signal(
        std::bind_front(&StatusBarController::on_total_update, this));
      m_realized_profit_and_loss->connect_update_signal(
        std::bind_front(&StatusBarController::on_total_update, this));
    }

    bool eventFilter(QObject* object, QEvent* event) override {
      if(event->type() == QEvent::Close) {
        QApplication::quit();
      }
      return QWidget::eventFilter(object, event);
    }

    void add_money_input(QGridLayout& layout, const QString& name, int row,
        QDoubleSpinBox*& box, std::shared_ptr<MoneyModel>& value) {
      layout.addWidget(new QLabel(name + ": "), row, 0);
      box = new QDoubleSpinBox();
      box->setMinimum(-999999999);
      box->setMaximum(99999999);
      box->setValue(0);
      layout.addWidget(box, row, 1);
      value = std::make_shared<LocalMoneyModel>();
      connect(box, &QDoubleSpinBox::textChanged,
        std::bind_front(&StatusBarController::on_money_update, this, value));
    }

    void on_money_update(
        std::shared_ptr<MoneyModel> value, const QString& text) {
      value->set(*Money::FromValue(text.toStdString()));
    }

    void on_total_update(Money value) {
      m_total_profit_and_loss->set(m_realized_profit_and_loss->get() +
        m_unrealized_profit_and_loss->get());
    }
  };
}

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Blotter UI Tester"));
  initialize_resources();
  auto controller = StatusBarController();
  auto tasks = std::make_shared<ArrayListModel<TaskEntry>>();
  auto blotter = std::make_shared<CompositeBlotterModel>(
    std::make_shared<LocalTextModel>("North America"),
    std::make_shared<LocalBooleanModel>(),
    std::make_shared<LocalBooleanModel>(), tasks,
    std::make_shared<LocalBlotterPositionsModel>(),
    std::make_shared<ArrayListModel<Order*>>(),
    std::make_shared<LocalBlotterProfitAndLossModel>(),
    std::make_shared<CompositeBlotterStatusModel>(controller.m_buying_power,
      controller.m_net_loss, controller.m_total_profit_and_loss,
      controller.m_unrealized_profit_and_loss,
      controller.m_realized_profit_and_loss, controller.m_fees,
      controller.m_cost_basis));
  controller.setAttribute(Qt::WA_ShowWithoutActivating);
  auto window = BlotterWindow(blotter);
  window.show();
  controller.move(window.pos() + QPoint(0, window.size().height()));
  controller.show();
  window.installEventFilter(&controller);
  tasks->push(
    TaskEntry(std::make_shared<Task>(), std::make_shared<LocalBooleanModel>()));
  application.exec();
}
