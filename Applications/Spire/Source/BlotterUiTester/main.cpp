#include <Beam/Queues/Queue.hpp>
#include <Aspen/Shared.hpp>
#include <QApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderReactor.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/Blotter/CompositeBlotterModel.hpp"
#include "Spire/Blotter/LocalValuationModel.hpp"
#include "Spire/Canvas/Task.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ConstantValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/QtTaskQueue.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DestinationBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/OrderTypeBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Ui/SideBox.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Spire;

namespace {
  auto get_security_info_list() {
    static auto info = [&] {
      auto info = std::vector<SecurityInfo>();
      info.emplace_back(ParseSecurity("A.NYSE"), "A Inc.", "", 0);
      info.emplace_back(ParseSecurity("B.NYSE"), "B Inc.", "", 0);
      info.emplace_back(ParseSecurity("C.NYSE"), "C Inc.", "", 0);
      info.emplace_back(ParseSecurity("D.TSX"), "D Inc.", "", 0);
      info.emplace_back(ParseSecurity("E.TSX"), "E Inc.", "", 0);
      info.emplace_back(ParseSecurity("F.TSX"), "F Inc.", "", 0);
      info.emplace_back(ParseSecurity("G.ASX"), "G Inc.", "", 0);
      info.emplace_back(ParseSecurity("H.ASX"), "H Inc.", "", 0);
      info.emplace_back(ParseSecurity("I.ASX"), "I Inc.", "", 0);
      return info;
    }();
    return info;
  }

  auto make_security_box() {
    auto model = std::make_shared<LocalComboBoxQueryModel>();
    for(auto& info : get_security_info_list()) {
      model->add(displayText(info.m_security).toLower(), info);
      model->add(QString::fromStdString(info.m_name).toLower(), info);
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

  struct ValuationWindow : QWidget {
    std::shared_ptr<LocalValuationModel> m_valuation;

    ValuationWindow(std::shared_ptr<LocalValuationModel> valuation,
        QWidget* parent = nullptr)
        : m_valuation(std::move(valuation)) {
      auto& markets = GetDefaultMarketDatabase();
      auto layout = new QGridLayout(this);
      auto row = 0;
      for(auto& info : get_security_info_list()) {
        layout->addWidget(make_label(
          QString::fromStdString(ToString(info.m_security))), row, 0);
        auto value = std::make_shared<LocalOptionalMoneyModel>(Money::ONE);
        layout->addWidget(new MoneyBox(value), row, 1);
        ++row;
        auto valuation = std::make_shared<CompositeModel<SecurityValuation>>();
        valuation->add(
          &SecurityValuation::m_currency, make_constant_value_model(
            markets.FromCode(info.m_security.GetMarket()).m_currency));
        valuation->add(&SecurityValuation::m_bidValue, value);
        valuation->add(&SecurityValuation::m_askValue, value);
        m_valuation->add(info.m_security, valuation);
      }
      resize(scale(230, 330));
    }
  };

  struct OrderEntryPanel : QWidget {
    TextBox* m_name_box;
    SecurityBox* m_security_box;
    DestinationBox* m_destination_box;
    OrderTypeBox* m_order_type_box;
    SideBox* m_side_box;
    QuantityBox* m_quantity_box;
    MoneyBox* m_price_box;
    Button* m_submit_button;

    OrderEntryPanel(QWidget* parent = nullptr)
        : QWidget(parent) {
      m_name_box = new TextBox("Test Order");
      m_security_box = make_security_box();
      m_security_box->get_current()->set(ParseSecurity("A.NYSE"));
      m_destination_box = make_destination_box();
      m_destination_box->get_current()->set("");
      m_order_type_box = make_order_type_box();
      m_side_box = make_side_box();
      m_quantity_box = new QuantityBox();
      m_quantity_box->get_current()->set(Quantity(100));
      m_price_box = new MoneyBox();
      m_price_box->get_current()->set(Money::ONE);
      m_submit_button = make_label_button("Submit");
      auto layout = new QGridLayout(this);
      layout->addWidget(make_label("Name"), 0, 0);
      layout->addWidget(make_label("Security"), 0, 1);
      layout->addWidget(make_label("Destination"), 0, 2);
      layout->addWidget(make_label("Order Type"), 0, 3);
      layout->addWidget(make_label("Side"), 0, 4);
      layout->addWidget(make_label("Quantity"), 0, 5);
      layout->addWidget(make_label("Price"), 0, 6);
      layout->addWidget(m_name_box, 1, 0);
      layout->addWidget(m_security_box, 1, 1);
      layout->addWidget(m_destination_box, 1, 2);
      layout->addWidget(m_order_type_box, 1, 3);
      layout->addWidget(m_side_box, 1, 4);
      layout->addWidget(m_quantity_box, 1, 5);
      layout->addWidget(m_price_box, 1, 6);
      layout->addWidget(m_submit_button, 1, 7);
    }

    QSize sizeHint() const {
      return QSize(scale_width(950), layout()->sizeHint().height());
    }

    bool eventFilter(QObject* object, QEvent* event) override {
      if(event->type() == QEvent::Close) {
        QApplication::quit();
      }
      return QWidget::eventFilter(object, event);
    }
  };

  struct ExecutionReportPanel : QWidget {
    CheckBox* m_auto_accept;
    CheckBox* m_auto_cancel;
    QuantityBox* m_last_quantity_box;
    MoneyBox* m_last_price_box;
    TextBox* m_market_box;
    TextBox* m_flag_box;
    TextBox* m_message_box;
    MoneyBox* m_execution_fee_box;
    MoneyBox* m_processing_fee_box;
    MoneyBox* m_miscellaneous_fee_box;
    Button* m_accept_button;
    Button* m_reject_button;
    Button* m_fill_button;
    Button* m_cancel_button;

    ExecutionReportPanel(QWidget* parent = nullptr)
        : QWidget(parent) {
      m_auto_accept = new CheckBox();
      m_auto_accept->set_label("Auto Accept");
      m_auto_accept->get_current()->set(true);
      m_auto_cancel = new CheckBox();
      m_auto_cancel->set_label("Auto Cancel");
      m_auto_cancel->get_current()->set(true);
      m_last_quantity_box = new QuantityBox();
      m_last_price_box = new MoneyBox();
      m_market_box = new TextBox();
      m_flag_box = new TextBox();
      m_message_box = new TextBox();
      m_execution_fee_box = new MoneyBox();
      m_processing_fee_box = new MoneyBox();
      m_miscellaneous_fee_box = new MoneyBox();
      m_accept_button = make_label_button("Accept");
      m_reject_button = make_label_button("Reject");
      m_fill_button = make_label_button("Fill");
      m_cancel_button = make_label_button("Cancel");
      auto layout = new QGridLayout(this);
      layout->addWidget(m_auto_accept, 0, 0);
      layout->addWidget(m_auto_cancel, 0, 1);
      layout->addWidget(make_label("Last Quantity"), 1, 0);
      layout->addWidget(make_label("Last Price"), 1, 1);
      layout->addWidget(make_label("Market"), 1, 2);
      layout->addWidget(make_label("Liquidity Flag"), 1, 3);
      layout->addWidget(make_label("Message"), 1, 4);
      layout->addWidget(make_label("Execution Fee"), 1, 5);
      layout->addWidget(make_label("Processing Fee"), 1, 6);
      layout->addWidget(make_label("Miscellaneous Fee"), 1, 7);
      layout->addWidget(m_last_quantity_box, 2, 0);
      layout->addWidget(m_last_price_box, 2, 1);
      layout->addWidget(m_market_box, 2, 2);
      layout->addWidget(m_flag_box, 2, 3);
      layout->addWidget(m_message_box, 2, 4);
      layout->addWidget(m_execution_fee_box, 2, 5);
      layout->addWidget(m_processing_fee_box, 2, 6);
      layout->addWidget(m_miscellaneous_fee_box, 2, 7);
      layout->addWidget(m_accept_button, 3, 0);
      layout->addWidget(m_reject_button, 3, 1);
      layout->addWidget(m_fill_button, 3, 2);
      layout->addWidget(m_cancel_button, 3, 3);
    }
  };

  struct ControlPanel : QWidget {
    OrderEntryPanel* m_order_entry_panel;
    ExecutionReportPanel* m_execution_report_panel;

    ControlPanel(QWidget* parent = nullptr)
        : QWidget(parent) {
      m_order_entry_panel = new OrderEntryPanel();
      m_execution_report_panel = new ExecutionReportPanel();
      auto layout = new QVBoxLayout(this);
      layout->addWidget(m_order_entry_panel);
      layout->addWidget(m_execution_report_panel);
    }

    bool eventFilter(QObject* receiver, QEvent* event) override {
      if(event->type() == QEvent::Close) {
        QApplication::closeAllWindows();
      }
      return QWidget::eventFilter(receiver, event);
    }
  };

  struct MockOrderExecutionClient {
    std::atomic<OrderId> m_next_id;
    std::unordered_set<std::unique_ptr<PrimitiveOrder>> m_orders;

    const Order& Submit(const OrderFields& fields) {
      auto id = ++m_next_id;
      auto proper_fields = fields;
      if(proper_fields.m_currency == CurrencyId::NONE) {
        proper_fields.m_currency = GetDefaultMarketDatabase().FromCode(
          proper_fields.m_security.GetMarket()).m_currency;
      }
      auto order = std::make_unique<PrimitiveOrder>(OrderInfo(
        std::move(proper_fields), id, second_clock::universal_time()));
      auto result = order.get();
      m_orders.insert(std::move(order));
      return *result;
    }

    void Cancel(const Order& order) {
      auto& base_order = const_cast<PrimitiveOrder&>(
        static_cast<const PrimitiveOrder&>(order));
      base_order.With([&] (auto current_status, auto& reports) {
        auto is_pending_cancel = false;
        for(auto& report : reports) {
          if(report.m_status == OrderStatus::PENDING_CANCEL) {
            is_pending_cancel = true;
            break;
          }
        }
        if(is_pending_cancel) {
          return;
        }
        auto& lastReport = reports.back();
        auto updatedReport = ExecutionReport::MakeUpdatedReport(lastReport,
          OrderStatus::PENDING_CANCEL, second_clock::universal_time());
        base_order.Update(updatedReport);
      });
    }
  };

  struct ObserverOrderExecutionClient {
    MockOrderExecutionClient* m_client;
    std::vector<const Order*> m_orders;
    std::shared_ptr<QueueWriter<const Order*>> m_submissions;

    ObserverOrderExecutionClient(MockOrderExecutionClient& client,
      std::shared_ptr<QueueWriter<const Order*>> submissions)
      : m_client(&client),
        m_submissions(std::move(submissions)) {}

    ~ObserverOrderExecutionClient() {
      for(auto order : m_orders) {
        Cancel(*order);
      }
    }

    const Order& Submit(const OrderFields& fields) {
      auto& order = m_client->Submit(fields);
      m_orders.push_back(&order);
      m_submissions->Push(&order);
      return order;
    }

    void Cancel(const Order& order) {
      m_client->Cancel(order);
    }
  };

  struct BlotterWindowController {
    int m_next_task_id;
    std::shared_ptr<BlotterModel> m_blotter;
    BlotterWindow* m_blotter_window;
    std::shared_ptr<LocalValuationModel> m_valuation;
    ValuationWindow* m_valuation_window;
    ControlPanel* m_control_panel;
    MockOrderExecutionClient m_client;
    QtTaskQueue m_tasks;

    BlotterWindowController(std::shared_ptr<BlotterModel> blotter,
        std::shared_ptr<LocalValuationModel> valuation)
        : m_next_task_id(1),
          m_blotter(std::move(blotter)),
          m_valuation(std::move(valuation)) {
      m_blotter_window = new BlotterWindow(m_blotter);
      m_blotter_window->show();
      m_control_panel = new ControlPanel();
      m_blotter_window->installEventFilter(m_control_panel);
      m_blotter_window->get_task_view().connect_execute_signal(
        std::bind_front(&BlotterWindowController::on_execute, this));
      m_blotter_window->get_task_view().connect_cancel_signal(
        std::bind_front(&BlotterWindowController::on_cancel_tasks, this));
      m_blotter_window->get_order_log_view().connect_cancel_signal(
        std::bind_front(&BlotterWindowController::on_cancel_orders_request,
          this));
      m_control_panel->setAttribute(Qt::WA_ShowWithoutActivating);
      m_control_panel->move(
        m_blotter_window->pos() + QPoint(0, m_blotter_window->size().height()));
      m_control_panel->show();
      m_control_panel->
        m_order_entry_panel->m_submit_button->connect_click_signal(
          std::bind_front(&BlotterWindowController::on_submit, this));
      m_control_panel->
        m_execution_report_panel->m_accept_button->connect_click_signal(
          std::bind_front(&BlotterWindowController::on_accept, this));
      m_control_panel->
        m_execution_report_panel->m_reject_button->connect_click_signal(
          std::bind_front(&BlotterWindowController::on_reject, this));
      m_control_panel->
        m_execution_report_panel->m_fill_button->connect_click_signal(
          std::bind_front(&BlotterWindowController::on_fill, this));
      m_control_panel->
        m_execution_report_panel->m_cancel_button->connect_click_signal(
          std::bind_front(&BlotterWindowController::on_cancel_orders, this));
      m_valuation_window = new ValuationWindow(m_valuation);
      m_valuation_window->setAttribute(Qt::WA_ShowWithoutActivating);
      m_valuation_window->show();
      m_valuation_window->move(m_blotter_window->pos().x() +
        m_blotter_window->frameGeometry().width(), m_blotter_window->pos().y());
    }

    std::tuple<Aspen::Box<void>,
        std::shared_ptr<SequencePublisher<const Order*>>> make_order_reactor() {
      auto& order_entry_panel = *m_control_panel->m_order_entry_panel;
      auto& security = order_entry_panel.m_security_box->get_current()->get();
      auto order_type =
        order_entry_panel.m_order_type_box->get_current()->get();
      auto side = order_entry_panel.m_side_box->get_current()->get();
      auto destination =
        order_entry_panel.m_destination_box->get_current()->get();
      auto quantity =
        order_entry_panel.m_quantity_box->get_current()->get().get_value_or(0);
      auto price =
        order_entry_panel.m_price_box->get_current()->get().get_value_or(
          Money::ZERO);
      auto submissions = std::make_shared<SequencePublisher<const Order*>>();
      auto client =
        std::make_unique<ObserverOrderExecutionClient>(m_client, submissions);
      return std::tuple(Aspen::Box<void>(
        OrderReactor(std::move(client), Aspen::constant(DirectoryEntry()),
          Aspen::constant(security), Aspen::constant(CurrencyId::NONE),
          Aspen::constant(order_type), Aspen::constant(side),
          Aspen::constant(destination), Aspen::constant(quantity),
          Aspen::constant(price),
          Aspen::constant(TimeInForce(TimeInForce::Type::GTC)),
          std::vector<Aspen::Box<Nexus::Tag>>())), std::move(submissions));
    }

    optional<Quantity> get_last_quantity() const {
      return m_control_panel->
        m_execution_report_panel->m_last_quantity_box->get_current()->get();
    }

    optional<Money> get_last_price() const {
      return m_control_panel->
        m_execution_report_panel->m_last_price_box->get_current()->get();
    }

    std::string get_market() const {
      return m_control_panel->m_execution_report_panel->
        m_market_box->get_current()->get().toStdString();
    }

    std::string get_flag() const {
      return m_control_panel->m_execution_report_panel->
        m_flag_box->get_current()->get().toStdString();
    }

    Money get_execution_fee() const {
      return m_control_panel->m_execution_report_panel->
        m_execution_fee_box->get_current()->get().value_or(Money::ZERO);
    }

    Money get_processing_fee() const {
      return m_control_panel->m_execution_report_panel->
        m_processing_fee_box->get_current()->get().value_or(Money::ZERO);
    }

    Money get_miscellaneous_fee() const {
      return m_control_panel->m_execution_report_panel->
        m_miscellaneous_fee_box->get_current()->get().value_or(Money::ZERO);
    }

    std::string get_message() const {
      return m_control_panel->m_execution_report_panel->
        m_message_box->get_current()->get().toStdString();
    }

    void on_submit() {
      auto [reactor, submissions] = make_order_reactor();
      submissions->Monitor(m_tasks.get_slot<const Order*>(
        std::bind_front(&BlotterWindowController::on_new_order, this)));
      auto task = std::make_unique<Task>(std::move(reactor));
      auto entry = std::make_shared<BlotterTaskEntry>(BlotterTaskEntry(
        m_next_task_id, m_control_panel->
          m_order_entry_panel->m_name_box->get_current()->get().toStdString(),
        false, std::move(task), std::move(submissions)));
      ++m_next_task_id;
      m_blotter->get_tasks()->push(entry);
    }

    void set_initial_status(OrderStatus status) {
      auto orders = m_blotter_window->get_order_log_view().get_selection();
      for(auto i = 0; i != orders->get_size(); ++i) {
        auto& order = *const_cast<PrimitiveOrder*>(
          static_cast<const PrimitiveOrder*>(orders->get(i)));
        order.With([&] (auto current_status, auto& reports) {
          if(reports.size() != 1) {
            return;
          }
          auto& lastReport = reports.back();
          auto updatedReport = ExecutionReport::MakeUpdatedReport(
            lastReport, status, second_clock::universal_time());
          updatedReport.m_text = get_message();
          order.Update(updatedReport);
        });
      }
    }

    void on_accept() {
      set_initial_status(OrderStatus::NEW);
    }

    void on_reject() {
      set_initial_status(OrderStatus::REJECTED);
    }

    void on_fill() {
      auto orders = m_blotter_window->get_order_log_view().get_selection();
      for(auto i = 0; i != orders->get_size(); ++i) {
        auto& order = *const_cast<PrimitiveOrder*>(
          static_cast<const PrimitiveOrder*>(orders->get(i)));
        order.With([&] (auto current_status, auto& reports) {
          if(IsTerminal(current_status)) {
            return;
          }
          auto fill = Quantity(0);
          for(auto& report : reports) {
            fill += report.m_lastQuantity;
          }
          auto next_fill = [&] {
            if(auto last_quantity = get_last_quantity()) {
              return std::min(
                *last_quantity, order.GetInfo().m_fields.m_quantity - fill);
            }
            return order.GetInfo().m_fields.m_quantity - fill;
          }();
          auto status = [&] {
            if(fill + next_fill == order.GetInfo().m_fields.m_quantity) {
              return OrderStatus::FILLED;
            }
            return OrderStatus::PARTIALLY_FILLED;
          }();
          auto& lastReport = reports.back();
          auto updatedReport = ExecutionReport::MakeUpdatedReport(
            lastReport, status, second_clock::universal_time());
          updatedReport.m_lastQuantity = next_fill;
          updatedReport.m_lastPrice =
            get_last_price().value_or(order.GetInfo().m_fields.m_price);
          updatedReport.m_lastMarket = get_market();
          updatedReport.m_liquidityFlag = get_flag();
          updatedReport.m_executionFee = get_execution_fee();
          updatedReport.m_processingFee = get_processing_fee();
          updatedReport.m_commission = get_miscellaneous_fee();
          updatedReport.m_text = get_message();
          order.Update(updatedReport);
        });
      }
    }

    void on_cancel_orders() {
      auto orders = m_blotter_window->get_order_log_view().get_selection();
      for(auto i = 0; i != orders->get_size(); ++i) {
        auto& order = *const_cast<PrimitiveOrder*>(
          static_cast<const PrimitiveOrder*>(orders->get(i)));
        order.With([&] (auto current_status, auto& reports) {
          if(IsTerminal(current_status)) {
            return;
          }
          auto is_pending_cancel = false;
          for(auto& report : reports) {
            if(report.m_status == OrderStatus::PENDING_CANCEL) {
              is_pending_cancel = true;
              break;
            }
          }
          if(!is_pending_cancel) {
            return;
          }
          auto& lastReport = reports.back();
          auto updatedReport = ExecutionReport::MakeUpdatedReport(
            lastReport, OrderStatus::CANCELED, second_clock::universal_time());
          updatedReport.m_text = get_message();
          order.Update(updatedReport);
        });
      }
    }

    void on_execute(
        const std::vector<std::shared_ptr<BlotterTaskEntry>>& tasks) {
      for(auto task : tasks) {
        task->m_task->execute();
      }
    }

    void on_cancel_tasks(
        const std::vector<std::shared_ptr<BlotterTaskEntry>>& tasks) {
      for(auto task : tasks) {
        task->m_task->cancel();
      }
    }

    void on_cancel_orders_request(const std::vector<const Order*>& orders) {
      for(auto& order : orders) {
        m_client.Cancel(*order);
      }
    }

    void on_new_order(const Order* order) {
      order->GetPublisher().Monitor(m_tasks.get_slot<ExecutionReport>(
        std::bind_front(&BlotterWindowController::on_execution_report, this,
          std::cref(*order))));
      auto auto_accept =
        m_control_panel->m_execution_report_panel->m_auto_accept;
      if(!auto_accept->get_current()->get()) {
        return;
      }
      Accept(*const_cast<PrimitiveOrder*>(static_cast<const PrimitiveOrder*>(
        order)), second_clock::universal_time());
    }

    void on_execution_report(
        const Order& order, const ExecutionReport& report) {
      auto auto_cancel =
        m_control_panel->m_execution_report_panel->m_auto_cancel;
      if(!auto_cancel->get_current()->get() ||
          report.m_status != OrderStatus::PENDING_CANCEL) {
        return;
      }
      auto& base_order =
        const_cast<PrimitiveOrder&>(static_cast<const PrimitiveOrder&>(order));
      base_order.With([&] (auto current_status, auto& reports) {
        auto& lastReport = reports.back();
        auto updatedReport = ExecutionReport::MakeUpdatedReport(
          lastReport, OrderStatus::CANCELED, second_clock::universal_time());
        base_order.Update(updatedReport);
      });
    }
  };
}

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Blotter UI Tester"));
  application.setQuitOnLastWindowClosed(true);
  initialize_resources();
  auto exchange_rates = ExchangeRateTable();
  auto tasks =
    std::make_shared<ArrayListModel<std::shared_ptr<BlotterTaskEntry>>>();
  auto valuation = std::make_shared<LocalValuationModel>();
  auto blotter = make_derived_blotter_model(
    std::make_shared<LocalTextModel>("North America"),
    std::make_shared<LocalBooleanModel>(),
    std::make_shared<LocalBooleanModel>(), GetDefaultMarketDatabase(),
    DefaultCurrencies::USD(), exchange_rates,
    std::make_shared<LocalMoneyModel>(), std::make_shared<LocalMoneyModel>(),
    tasks, valuation);
  auto controller = BlotterWindowController(blotter, valuation);
  application.exec();
}
