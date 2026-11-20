#include "Spire/BlotterUiTester/BlotterController.hpp"
#include <stdexcept>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QVBoxLayout>
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/ValueNodes/CurrencyNode.hpp"
#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderTypeNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TickerNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

BlotterController::BlotterController(std::shared_ptr<TasksModel> tasks,
    Ref<TestEnvironment> environment, Ref<UserProfile> profile)
    : m_tasks(std::move(tasks)),
      m_environment(environment.get()),
      m_profile(profile.get()),
      m_name(new QLineEdit(tr("Single Order"))),
      m_ticker(new QLineEdit("MRU.TSX")),
      m_currency(new QLineEdit("CAD")),
      m_type(new QComboBox()),
      m_side(new QComboBox()),
      m_destination(new QLineEdit("TSX")),
      m_quantity(new QSpinBox()),
      m_price(new QLineEdit("10.00")),
      m_time_in_force(new QComboBox()),
      m_selection(new QComboBox()),
      m_status(new QLabel()),
      m_fill_price(new QLineEdit("10.00")),
      m_fill_quantity(new QSpinBox()),
      m_accept(new QPushButton(tr("New"))),
      m_reject(new QPushButton(tr("Reject"))),
      m_fill(new QPushButton(tr("Fill Remaining"))),
      m_partial_fill(new QPushButton(tr("Partial Fill"))),
      m_cancel(new QPushButton(tr("Cancel"))),
      m_output(new QTextEdit()) {
  setWindowTitle(tr("Blotter Controller"));
  auto layout = new QVBoxLayout(this);
  auto control_row = new QHBoxLayout();
  layout->addLayout(control_row);
  auto creation = new QGroupBox(tr("Create Task"));
  auto form = new QFormLayout(creation);
  form->addRow(tr("Name"), m_name);
  form->addRow(tr("Ticker"), m_ticker);
  form->addRow(tr("Currency"), m_currency);
  for(auto type : {OrderType::LIMIT, OrderType::MARKET, OrderType::PEGGED,
      OrderType::STOP}) {
    m_type->addItem(to_text(OrderType(type)), static_cast<int>(type));
  }
  form->addRow(tr("Type"), m_type);
  for(auto side : {Side::BID, Side::ASK}) {
    m_side->addItem(to_text(Side(side)), static_cast<int>(side));
  }
  form->addRow(tr("Side"), m_side);
  form->addRow(tr("Destination"), m_destination);
  m_quantity->setRange(1, 1000000000);
  m_quantity->setValue(100);
  form->addRow(tr("Quantity"), m_quantity);
  form->addRow(tr("Price"), m_price);
  for(auto type : {TimeInForce::Type::DAY, TimeInForce::Type::GTC,
      TimeInForce::Type::OPG, TimeInForce::Type::IOC, TimeInForce::Type::FOK,
      TimeInForce::Type::GTX, TimeInForce::Type::MOC}) {
    m_time_in_force->addItem(
      to_text(TimeInForce(type)), static_cast<int>(type));
  }
  form->addRow(tr("Time in Force"), m_time_in_force);
  auto account =
    m_profile->GetClients().get_service_locator_client().get_account();
  form->addRow(
    tr("Account"), new QLabel(QString::fromStdString(account.m_name)));
  auto create = new QPushButton(tr("Create Task"));
  connect(create, &QPushButton::clicked, this, &BlotterController::on_create);
  form->addRow(create);
  control_row->addWidget(creation);
  auto execution = new QGroupBox(tr("Control Order"));
  auto controls = new QVBoxLayout(execution);
  controls->addWidget(m_selection);
  controls->addWidget(m_status);
  m_status->setWordWrap(true);
  auto initial = new QHBoxLayout();
  initial->addWidget(m_accept);
  initial->addWidget(m_reject);
  controls->addLayout(initial);
  auto fills = new QFormLayout();
  fills->addRow(tr("Fill Price"), m_fill_price);
  m_fill_quantity->setRange(1, 1000000000);
  m_fill_quantity->setValue(50);
  fills->addRow(tr("Fill Quantity"), m_fill_quantity);
  controls->addLayout(fills);
  controls->addWidget(m_partial_fill);
  controls->addWidget(m_fill);
  controls->addWidget(m_cancel);
  controls->addStretch();
  control_row->addWidget(execution);
  m_output->setReadOnly(true);
  m_output->setPlaceholderText(tr("Signal output"));
  layout->addWidget(m_output, 1);
  connect(m_selection, qOverload<int>(&QComboBox::currentIndexChanged), this,
    &BlotterController::on_selection);
  connect(m_accept, &QPushButton::clicked, this, [=, this] {
    on_action(OrderStatus::NEW);
  });
  connect(m_reject, &QPushButton::clicked, this, [=, this] {
    on_action(OrderStatus::REJECTED);
  });
  connect(m_fill, &QPushButton::clicked, this, [=, this] {
    on_action(OrderStatus::FILLED);
  });
  connect(m_partial_fill, &QPushButton::clicked, this, [=, this] {
    on_action(OrderStatus::PARTIALLY_FILLED);
  });
  connect(m_cancel, &QPushButton::clicked, this, [=, this] {
    on_action(OrderStatus::CANCELED);
  });
  m_environment->monitor_order_submissions(
    m_events.get_slot<std::shared_ptr<Order>>(
      std::bind_front(&BlotterController::on_submission, this)));
  refresh();
  resize(scale(660, 540));
}

BlotterController::~BlotterController() {
  m_events.close();
  for(auto& entry : m_entries) {
    entry.m_task->GetExecutor().Close();
  }
  flush_pending_routines();
}

void BlotterController::on_cancel_tasks(
    const std::vector<std::shared_ptr<Task>>& tasks) {
  auto ids = QStringList();
  for(auto& task : tasks) {
    ids.push_back(QString::number(task->GetId()));
  }
  m_output->append(tr("Cancel Tasks: [%1]").arg(ids.join(", ")));
}

OrderFields BlotterController::make_fields() const {
  auto fields = OrderFields();
  fields.m_account =
    m_profile->GetClients().get_service_locator_client().get_account();
  fields.m_ticker =
    parse_ticker(m_ticker->text().trimmed().toUpper().toStdString());
  if(!fields.m_ticker) {
    throw std::runtime_error("Enter a ticker and venue, such as MRU.TSX.");
  }
  fields.m_currency =
    parse_currency(m_currency->text().trimmed().toUpper().toStdString());
  if(!fields.m_currency) {
    throw std::runtime_error("Enter a recognized currency, such as CAD.");
  }
  fields.m_type = static_cast<OrderType>(m_type->currentData().toInt());
  fields.m_side = static_cast<Side>(m_side->currentData().toInt());
  fields.m_destination =
    m_destination->text().trimmed().toUpper().toStdString();
  if(fields.m_destination.empty()) {
    throw std::runtime_error("Enter an order destination.");
  }
  fields.m_quantity = m_quantity->value();
  fields.m_price = parse_money(m_price->text().trimmed().toStdString());
  if(fields.m_price < Money::ZERO) {
    throw std::runtime_error("Order price must not be negative.");
  }
  fields.m_time_in_force = TimeInForce(
    static_cast<TimeInForce::Type>(m_time_in_force->currentData().toInt()));
  return fields;
}

void BlotterController::refresh() {
  m_accept->setEnabled(false);
  m_reject->setEnabled(false);
  m_fill->setEnabled(false);
  m_partial_fill->setEnabled(false);
  m_cancel->setEnabled(false);
  m_fill_price->setEnabled(false);
  m_fill_quantity->setEnabled(false);
  auto index = m_selection->currentIndex();
  if(index < 0) {
    m_status->setText(tr("Create a task to control its order."));
    return;
  }
  auto& entry = m_entries[index];
  if(!entry.m_order) {
    m_status->setText(tr("Task: %1\n%2").
      arg(to_text(entry.m_state.m_state)).
      arg(QString::fromStdString(entry.m_state.m_message)));
    return;
  }
  auto remaining =
    entry.m_order->get_info().m_fields.m_quantity - entry.m_filled;
  m_status->setText(tr("Order %1: %2\nFilled: %3   Remaining: %4").
    arg(QString::number(entry.m_order->get_info().m_id)).
    arg(to_text(entry.m_status)).arg(to_text(entry.m_filled)).
    arg(to_text(remaining)));
  if(entry.m_is_pending ||
      !m_submissions.contains(entry.m_order->get_info().m_id)) {
    return;
  }
  auto pending = entry.m_status == OrderStatus::PENDING_NEW;
  auto active = entry.m_status ==
    OrderStatus::NEW || entry.m_status == OrderStatus::PARTIALLY_FILLED;
  m_accept->setEnabled(pending);
  m_reject->setEnabled(pending);
  m_fill->setEnabled(active);
  m_partial_fill->setEnabled(active && remaining > 1);
  m_cancel->setEnabled(active);
  m_fill_price->setEnabled(active);
  m_fill_quantity->setEnabled(active && remaining > 1);
  if(active && remaining > 1) {
    m_fill_quantity->setMaximum(static_cast<int>(remaining) - 1);
  }
}

void BlotterController::on_create() {
  try {
    auto fields = make_fields();
    auto name = m_name->text().trimmed();
    if(name.isEmpty()) {
      throw std::runtime_error("Enter a task name.");
    }
    auto builder = CanvasNodeBuilder(SingleOrderTaskNode(name.toStdString()));
    builder.Replace(SingleOrderTaskNode::TICKER_PROPERTY,
      std::make_unique<TickerNode>(fields.m_ticker));
    builder.Replace(SingleOrderTaskNode::CURRENCY_PROPERTY,
      std::make_unique<CurrencyNode>(
        fields.m_currency, to_text(fields.m_currency).toStdString()));
    builder.Replace(SingleOrderTaskNode::ORDER_TYPE_PROPERTY,
      std::make_unique<OrderTypeNode>(fields.m_type));
    builder.Replace(SingleOrderTaskNode::SIDE_PROPERTY,
      std::make_unique<SideNode>(fields.m_side));
    builder.Replace(SingleOrderTaskNode::DESTINATION_PROPERTY,
      std::make_unique<DestinationNode>(fields.m_destination));
    builder.Replace(SingleOrderTaskNode::QUANTITY_PROPERTY,
      std::make_unique<IntegerNode>(fields.m_quantity));
    builder.Replace(SingleOrderTaskNode::PRICE_PROPERTY,
      std::make_unique<MoneyNode>(fields.m_price));
    builder.Replace(SingleOrderTaskNode::TIME_IN_FORCE_PROPERTY,
      std::make_unique<TimeInForceNode>(fields.m_time_in_force));
    auto task = std::make_shared<Task>(
      *builder.Make(), fields.m_account, Ref(*m_profile));
    auto index = static_cast<int>(m_entries.size());
    auto entry = Entry();
    entry.m_task = task;
    m_entries.push_back(std::move(entry));
    auto blotter_entry = BlotterTaskEntry();
    blotter_entry.m_task = task;
    m_tasks->push(blotter_entry);
    task->GetPublisher().monitor(m_events.get_slot<Task::StateEntry>(
      std::bind_front(&BlotterController::on_state, this, index)));
    task->GetContext().GetOrderPublisher().monitor(
      m_events.get_slot<std::shared_ptr<Order>>(
        std::bind_front(&BlotterController::on_order, this, index)));
    m_selection->addItem(tr("%1: %2").arg(task->GetId()).arg(name));
    m_selection->setCurrentIndex(index);
    try {
      task->Execute();
    } catch(const std::exception& e) {
      on_state(index, Task::StateEntry(Task::State::FAILED, e.what()));
    }
  } catch(const std::exception& e) {
    QMessageBox::warning(this, tr("Unable to Create Task"), e.what());
  }
}

void BlotterController::on_selection(int index) {
  if(index >= 0 && m_entries[index].m_order) {
    m_fill_price->setText(
      to_text(m_entries[index].m_order->get_info().m_fields.m_price));
  }
  refresh();
}

void BlotterController::on_submission(const std::shared_ptr<Order>& order) {
  m_submissions.emplace(order->get_info().m_id, order);
  refresh();
}

void BlotterController::on_order(
    int index, const std::shared_ptr<Order>& order) {
  m_entries[index].m_order = order;
  order->get_publisher().monitor(m_events.get_slot<ExecutionReport>(
    std::bind_front(&BlotterController::on_report, this, index)));
  if(index == m_selection->currentIndex()) {
    on_selection(index);
  }
}

void BlotterController::on_report(int index, const ExecutionReport& report) {
  auto& entry = m_entries[index];
  entry.m_status = report.m_status;
  entry.m_filled += report.m_last_quantity;
  entry.m_is_pending = false;
  refresh();
}

void BlotterController::on_state(int index, const Task::StateEntry& state) {
  m_entries[index].m_state = state;
  for(auto i = 0; i < m_tasks->get_size(); ++i) {
    auto entry = m_tasks->get(i);
    if(entry.m_task == m_entries[index].m_task) {
      entry.m_state = state.m_state;
      m_tasks->set(i, entry);
      break;
    }
  }
  refresh();
}

void BlotterController::on_action(OrderStatus status) {
  auto& entry = m_entries[m_selection->currentIndex()];
  auto& order = *m_submissions.at(entry.m_order->get_info().m_id);
  try {
    entry.m_is_pending = true;
    refresh();
    m_environment->set(boost::posix_time::microsec_clock::universal_time());
    if(status == OrderStatus::NEW) {
      m_environment->accept(order);
    } else if(status == OrderStatus::REJECTED) {
      m_environment->reject(order);
    } else if(status == OrderStatus::CANCELED) {
      m_profile->GetClients().get_order_execution_client().cancel(
        entry.m_order);
      m_environment->cancel(order);
    } else {
      auto price = parse_money(m_fill_price->text().trimmed().toStdString());
      if(price <= Money::ZERO) {
        throw std::runtime_error("Fill price must be positive.");
      }
      auto remaining = order.get_info().m_fields.m_quantity - entry.m_filled;
      auto quantity = remaining;
      if(status == OrderStatus::PARTIALLY_FILLED) {
        quantity = m_fill_quantity->value();
        if(quantity <= 0 || quantity >= remaining) {
          throw std::runtime_error(
            "Partial fill quantity must be less than the remaining quantity.");
        }
      }
      m_environment->fill(order, price, quantity);
    }
  } catch(const std::exception& e) {
    entry.m_is_pending = false;
    refresh();
    QMessageBox::warning(this, tr("Unable to Update Order"), e.what());
  }
}
