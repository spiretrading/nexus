#include <QApplication>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Blotter/CompositeBlotterStatusModel.hpp"
#include "Spire/Blotter/LocalBlotterPositionsModel.hpp"
#include "Spire/Blotter/LocalBlotterProfitAndLossModel.hpp"
#include "Spire/Canvas/Task.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DestinationBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/OrderTypeBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Ui/SideBox.hpp"
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

  auto make_security_box() {
    auto security_infos = std::vector<SecurityInfo>();
    security_infos.emplace_back(
      ParseSecurity("MSFT.NSDQ"), "Microsoft Inc.", "", 0);
    security_infos.emplace_back(ParseSecurity("MRU.TSX"), "Metro Inc.", "", 0);
    security_infos.emplace_back(
      ParseSecurity("MG.TSX"), "Magna International Inc.", "", 0);
    security_infos.emplace_back(
      ParseSecurity("MGA.TSX"), "Mega Uranium Ltd.", "", 0);
    security_infos.emplace_back(ParseSecurity("MGAB.TSX"),
      "Mackenzie Global Fixed Income Alloc ETF", "", 0);
    security_infos.emplace_back(
      ParseSecurity("MON.NYSE"), "Monsanto Co.", "", 0);
    security_infos.emplace_back(
      ParseSecurity("MFC.TSX"), "Manulife Financial Corporation", "", 0);
    security_infos.emplace_back(
      ParseSecurity("MX.TSX"), "Methanex Corporation", "", 0);
    auto model = std::make_shared<LocalComboBoxQueryModel>();
    for(auto& security_info : security_infos) {
      model->add(
        displayText(security_info.m_security).toLower(), security_info);
      model->add(
        QString::fromStdString(security_info.m_name).toLower(), security_info);
    }
    return new SecurityBox(model);
  }

  auto make_destination_box() {
    auto destinations = GetDefaultDestinationDatabase().SelectEntries(
      [] (auto& value) { return true; });
    auto model = std::make_shared<LocalComboBoxQueryModel>();
    for(auto destination : destinations) {
      model->add(displayText(destination.m_id).toLower(), destination);
    }
    return new DestinationBox(model);
  }

  struct OrderEntryPanel : QWidget {
    int m_next_id;
    std::shared_ptr<ListModel<BlotterTaskEntry>> m_tasks;
    SecurityBox* m_security_box;
    DestinationBox* m_destination_box;
    OrderTypeBox* m_order_type_box;
    SideBox* m_side_box;
    QuantityBox* m_quantity_box;
    MoneyBox* m_money_box;
    Button* m_submit_button;

    OrderEntryPanel(std::shared_ptr<ListModel<BlotterTaskEntry>> tasks,
        QWidget* parent = nullptr)
        : QWidget(parent),
          m_next_id(1),
          m_tasks(std::move(tasks)) {
      m_security_box = make_security_box();
      m_security_box->get_current()->set(ParseSecurity("MSFT.NSDQ"));
      m_destination_box = make_destination_box();
      m_destination_box->get_current()->set("NASDAQ");
      m_order_type_box = make_order_type_box();
      m_side_box = make_side_box();
      m_quantity_box = new QuantityBox();
      m_quantity_box->get_current()->set(Quantity(100));
      m_money_box = new MoneyBox();
      m_money_box->get_current()->set(Money::ONE);
      m_submit_button = make_label_button("Submit");
      auto layout = new QHBoxLayout(this);
      layout->addWidget(m_security_box);
      layout->addWidget(m_destination_box);
      layout->addWidget(m_order_type_box);
      layout->addWidget(m_side_box);
      layout->addWidget(m_quantity_box);
      layout->addWidget(m_money_box);
      layout->addWidget(m_submit_button);
      m_submit_button->connect_click_signal(
        std::bind_front(&OrderEntryPanel::on_submit, this));
    }

    void on_submit() {
    }
  };
}

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Blotter UI Tester"));
  initialize_resources();
  auto controller = StatusBarController();
  auto tasks = std::make_shared<ArrayListModel<BlotterTaskEntry>>();
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
  auto order_entry_panel = OrderEntryPanel(tasks);
  order_entry_panel.move(window.pos() + QPoint(0, window.size().height()));
  order_entry_panel.show();
  order_entry_panel.resize(scale(500, 50));
  window.installEventFilter(&controller);
  application.exec();
}
