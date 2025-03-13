#include "Spire/BookView/BookViewWindow.hpp"
#include <QKeyEvent>
#include <QScreen>
#include "Spire/BookView/TechnicalsPanel.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/SystemNodes/InteractionsNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/CanvasView/CanvasNodeNotVisibleException.hpp"
#include "Spire/CanvasView/CondensedCanvasWidget.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/SecurityView.hpp"
#include "Spire/Ui/TransitionView.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto TITLE_NAME = QObject::tr("Book View");
}

BookViewWindow::BookViewWindow(Ref<UserProfile> user_profile,
    std::shared_ptr<SecurityInfoQueryModel> securities,
    std::shared_ptr<KeyBindingsModel> key_bindings, MarketDatabase markets,
    std::shared_ptr<BookViewPropertiesWindowFactory> factory,
    ModelBuilder model_builder, QWidget* parent)
    : Window(parent),
      m_user_profile(user_profile.Get()),
      m_key_bindings(std::move(key_bindings)),
      m_markets(std::move(markets)),
      m_factory(std::move(factory)),
      m_model_builder(std::move(model_builder)),
      m_task_entry_panel(nullptr),
      m_is_task_entry_panel_for_interactions(false) {
  set_svg_icon(":/Icons/bookview.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/bookview.png"));
  setWindowTitle(TITLE_NAME);
  m_transition_view = new TransitionView(new QWidget());
  m_security_view = new SecurityView(std::move(securities), *m_transition_view);
  m_security_view->get_current()->connect_update_signal(
    std::bind_front(&BookViewWindow::on_current, this));
  m_security_view->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  set_body(m_security_view);
  update_style(*this, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  resize(scale(266, 361));
}

connection BookViewWindow::connect_submit_task_signal(
    const SubmitTaskSignal::slot_type& slot) const {
  return m_submit_task_signal.connect(slot);
}

connection BookViewWindow::connect_cancel_order_signal(
    const CancelOrderSignal::slot_type& slot) const {
  return m_cancel_order_signal.connect(slot);
}

void BookViewWindow::keyPressEvent(QKeyEvent* event) {
  auto sequence = QKeySequence(event->modifiers() | event->key());
  if(m_task_entry_panel) {
    on_task_entry_key_press(*event);
  } else if(
      m_selected_quote && sequence == QKeySequence(Qt::CTRL + Qt::Key_K)) {
    if(auto selected_quote = m_selected_quote->get()) {
      on_cancel_most_recent(*selected_quote);
    }
  } else if(m_selected_quote &&
      sequence == QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_K)) {
    if(auto selected_quote = m_selected_quote->get()) {
      on_cancel_all(*selected_quote);
    }
  } else if(sequence.matches(Qt::Key_QuoteLeft) == QKeySequence::ExactMatch) {
    display_interactions_panel();
  } else if(auto operation =
      m_key_bindings->get_cancel_key_bindings()->find_operation(sequence)) {
    m_cancel_order_signal(
      *operation, m_security_view->get_current()->get(), none);
  } else if(auto arguments = find_order_task_arguments(
      *m_key_bindings->get_order_task_arguments(),
      m_security_view->get_current()->get(), sequence)) {
    display_task_entry_panel(*arguments);
  }
}

std::unique_ptr<CanvasNode>
    BookViewWindow::make_task_node(const CanvasNode& node) {
  auto task_node = CanvasNode::Clone(node);
  auto security_node =
    task_node->FindNode(SingleOrderTaskNode::SECURITY_PROPERTY);
  if(security_node && !security_node->IsReadOnly()) {
    auto security = m_security_view->get_current()->get();
    if(auto security_value_node =
        dynamic_cast<const SecurityNode*>(&*security_node)) {
      auto builder = CanvasNodeBuilder(*task_node);
      builder.Replace(
        *security_node, security_value_node->SetValue(security, m_markets));
      builder.SetReadOnly(*security_node, true);
      auto price_node =
        task_node->FindNode(SingleOrderTaskNode::PRICE_PROPERTY);
      if(price_node && !price_node->IsReadOnly()) {
        auto price = [&] {
          if(auto side_node =
              task_node->FindNode(SingleOrderTaskNode::SIDE_PROPERTY)) {
            if(auto side_value_node =
                dynamic_cast<const SideNode*>(&*side_node)) {
              if(side_value_node->GetValue() == Side::ASK) {
                return m_model->get_bbo_quote()->get().m_ask.m_price;
              }
            }
          }
          return m_model->get_bbo_quote()->get().m_bid.m_price;
        }();
        if(auto money_node = dynamic_cast<const MoneyNode*>(&*price_node)) {
          builder.Replace(*price_node, money_node->SetValue(price));
        } else if(auto money_node =
            dynamic_cast<const OptionalPriceNode*>(&*price_node)) {
          builder.Replace(*price_node, money_node->SetReferencePrice(price));
        }
      }
      auto quantity_node =
        task_node->FindNode(SingleOrderTaskNode::QUANTITY_PROPERTY);
      if(quantity_node && !quantity_node->IsReadOnly()) {
        if(auto quantity_value_node =
            dynamic_cast<const IntegerNode*>(&*quantity_node)) {
          auto quantity = [&] {
            auto side_node =
              task_node->FindNode(SingleOrderTaskNode::SIDE_PROPERTY);
            auto& interactions =
              *m_key_bindings->get_interactions_key_bindings(security);
            if(side_node) {
              if(auto side_value_node =
                  dynamic_cast<const SideNode*>(&*side_node)) {
                return get_default_order_quantity(
                  *m_user_profile, security, side_value_node->GetValue());
              }
            }
            return interactions.get_default_quantity()->get();
          }();
          builder.Replace(*quantity_node,
            quantity_value_node->SetValue(static_cast<int>(quantity)));
        }
      }
      task_node = builder.Make();
    }
  }
  return task_node;
}

void BookViewWindow::display_interactions_panel() {
  auto security = m_security_view->get_current()->get();
  auto& interactions = *m_key_bindings->get_interactions_key_bindings(security);
  auto interactions_node = InteractionsNode(security, m_markets, interactions);
  m_task_entry_panel =
    new CondensedCanvasWidget("Interactions", Ref(*m_user_profile), this);
  m_is_task_entry_panel_for_interactions = true;
  auto coordinate = CanvasNodeModel::Coordinate(0, 0);
  m_task_entry_panel->Add(coordinate, interactions_node);
  m_task_entry_panel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  m_transition_view->layout()->addWidget(m_task_entry_panel);
  m_task_entry_panel->Focus();
}

void BookViewWindow::display_task_entry_panel(
    const OrderTaskArguments& arguments) {
  auto task_node = make_task_node(
    *make_canvas_node(arguments, m_user_profile->GetAdditionalTagDatabase()));
  m_task_entry_panel = new CondensedCanvasWidget(
    arguments.m_name.toStdString(), Ref(*m_user_profile), this);
  auto coordinate = CanvasNodeModel::Coordinate(0, 0);
  auto isVisible = [&] {
    try {
      m_task_entry_panel->Add(coordinate, *task_node);
      return true;
    } catch(const CanvasNodeNotVisibleException&) {
      return false;
    }
  }();
  if(isVisible) {
    m_task_entry_panel->setSizePolicy(
      QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_transition_view->layout()->addWidget(m_task_entry_panel);
    m_task_entry_panel->Focus();
  } else {
    m_task_entry_panel->deleteLater();
    m_task_entry_panel = nullptr;
    auto signaled_task = std::shared_ptr(std::move(task_node));
    m_submit_task_signal(signaled_task);
  }
}

void BookViewWindow::remove_task_entry_panel() {
  setUpdatesEnabled(false);
  m_transition_view->layout()->removeWidget(m_task_entry_panel);
  m_task_entry_panel->deleteLater();
  m_task_entry_panel = nullptr;
  m_is_task_entry_panel_for_interactions = false;
  setUpdatesEnabled(true);
}

void BookViewWindow::on_context_menu(
    MarketDepth* market_depth, const QPoint& pos) {
  auto menu = new ContextMenu(*market_depth);
  if(auto selected_quote = market_depth->get_selected_book_quote()->get()) {
    menu->add_action(tr("Cancel Most Recent"),
      std::bind_front(
        &BookViewWindow::on_cancel_most_recent, this, *selected_quote));
    menu->add_action(tr("Cancel All"),
      std::bind_front(&BookViewWindow::on_cancel_all, this, *selected_quote));
    menu->add_separator();
  }
  menu->add_action(tr("Properties"),
    std::bind_front(&BookViewWindow::on_properties_menu, this));
  menu->window()->setAttribute(Qt::WA_DeleteOnClose);
  menu->window()->move(market_depth->mapToGlobal(pos));
  menu->window()->show();
}

void BookViewWindow::on_task_entry_key_press(const QKeyEvent& event) {
  auto base_key = event.key();
  if(base_key == Qt::Key_Escape) {
    remove_task_entry_panel();
  } else if(base_key == Qt::Key_Enter || base_key == Qt::Key_Return) {
    if(m_is_task_entry_panel_for_interactions) {
      auto& node = static_cast<const InteractionsNode&>(
        *m_task_entry_panel->GetRoots().front());
      apply(node, *m_key_bindings);
      remove_task_entry_panel();
    } else {
      auto task_node = std::shared_ptr(
        CanvasNode::Clone(*m_task_entry_panel->GetRoots().front()));
      remove_task_entry_panel();
      m_submit_task_signal(task_node);
    }
  } else if(base_key == Qt::Key_QuoteLeft) {
    remove_task_entry_panel();
    display_interactions_panel();
  } else {
    auto sequence =
      QKeySequence(static_cast<int>(event.modifiers() + event.key()));
    if(auto arguments = find_order_task_arguments(
        *m_key_bindings->get_order_task_arguments(),
        m_security_view->get_current()->get(), sequence)) {
      remove_task_entry_panel();
      display_task_entry_panel(*arguments);
    }
  }
}

void BookViewWindow::on_cancel_most_recent(const BookQuote& book_quote) {
  auto operation = [&] {
    if(book_quote.m_quote.m_side == Side::BID) {
      return CancelKeyBindingsModel::Operation::MOST_RECENT_BID;
    }
    return CancelKeyBindingsModel::Operation::MOST_RECENT_ASK;
  }();
  m_cancel_order_signal(operation, m_security_view->get_current()->get(),
    std::tuple(book_quote.m_mpid.substr(1), book_quote.m_quote.m_price));
}

void BookViewWindow::on_cancel_all(const BookQuote& book_quote) {
  auto operation = [&] {
    if(book_quote.m_quote.m_side == Side::BID) {
      return CancelKeyBindingsModel::Operation::ALL_BIDS;
    }
    return CancelKeyBindingsModel::Operation::ALL_ASKS;
  }();
  m_cancel_order_signal(operation, m_security_view->get_current()->get(),
    std::tuple(book_quote.m_mpid.substr(1), book_quote.m_quote.m_price));
}

void BookViewWindow::on_properties_menu() {
  auto properties_window = m_factory->make(
    m_key_bindings, m_security_view->get_current()->get(), m_markets);
  if(!properties_window->isVisible()) {
    properties_window->show();
    if(screen()->geometry().right() - frameGeometry().right() >=
        properties_window->frameGeometry().width()) {
      properties_window->move(frameGeometry().right(), y());
    } else {
      properties_window->move(
        x() - properties_window->frameGeometry().width(), y());
    }
  }
  properties_window->activateWindow();
}

void BookViewWindow::on_current(const Security& security) {
  setWindowTitle(to_text(security) + " " + QString(0x2013) + " " + TITLE_NAME);
  m_transition_view->set_status(TransitionView::Status::NONE);
  m_interactions = m_key_bindings->get_interactions_key_bindings(security);
  m_model = m_model_builder(security);
  auto body = new QWidget();
  auto layout = make_vbox_layout(body);
  auto panel = new TechnicalsPanel(m_model->get_technicals(),
    m_interactions->get_default_quantity(),
    m_interactions->get_default_quantity());
  panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  layout->addWidget(panel);
  if(security == Security()) {
    layout->addStretch(1);
    m_selected_quote.reset();
  } else {
    auto market_depth = new MarketDepth(m_model, m_model->get_bbo_quote(),
      m_factory->get_properties(), m_markets);
    market_depth->setContextMenuPolicy(Qt::CustomContextMenu);
    m_selected_quote = market_depth->get_selected_book_quote();
    layout->addWidget(market_depth);
    body->setFocusProxy(market_depth);
    connect(market_depth, &QWidget::customContextMenuRequested,
      std::bind_front(&BookViewWindow::on_context_menu, this, market_depth));
  }
  m_transition_view->set_body(*body);
  m_transition_view->set_status(TransitionView::Status::READY);
  m_bid_order_connection = m_model->get_bid_orders()->connect_operation_signal(
    std::bind_front(&BookViewWindow::on_order_operation, this, Side::BID));
  m_ask_order_connection = m_model->get_ask_orders()->connect_operation_signal(
    std::bind_front(&BookViewWindow::on_order_operation, this, Side::ASK));
}

void BookViewWindow::on_order_operation(Side side,
    const ListModel<BookViewModel::UserOrder>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<BookViewModel::UserOrder>::UpdateOperation&
        operation) {
      auto cancel_operation = [&] {
        if(side == Side::BID) {
          return CancelKeyBindingsModel::Operation::ALL_BIDS;
        }
        return CancelKeyBindingsModel::Operation::ALL_ASKS;
      }();
      if(operation.get_value().m_status == OrderStatus::FILLED &&
          m_interactions->is_cancel_on_fill()->get()) {
        m_cancel_order_signal(
          cancel_operation, m_security_view->get_current()->get(), none);
      }
    });
}
