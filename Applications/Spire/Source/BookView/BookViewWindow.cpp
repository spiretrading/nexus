#include "Spire/BookView/BookViewWindow.hpp"
#include <QKeyEvent>
#include <QScreen>
#include "Spire/BookView/TechnicalsPanel.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/SecurityView.hpp"
#include "Spire/Ui/TransitionView.hpp"
#include "Spire/Utilities/LinkMenu.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto TITLE_NAME = QObject::tr("Book View");
}

BookViewWindow::BookViewWindow(
    std::shared_ptr<SecurityInfoQueryModel> securities,
    std::shared_ptr<KeyBindingsModel> key_bindings, MarketDatabase markets,
    std::shared_ptr<BookViewPropertiesWindowFactory> factory,
    ModelBuilder model_builder, QWidget* parent)
    : Window(parent),
      m_key_bindings(std::move(key_bindings)),
      m_markets(std::move(markets)),
      m_factory(std::move(factory)),
      m_model_builder(std::move(model_builder)),
      m_default_bid_quantity(std::make_shared<LocalQuantityModel>()),
      m_default_ask_quantity(std::make_shared<LocalQuantityModel>()) {
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

connection BookViewWindow::connect_cancel_order_signal(
    const CancelOrderSignal::slot_type& slot) const {
  return m_cancel_order_signal.connect(slot);
}

void BookViewWindow::keyPressEvent(QKeyEvent* event) {
  auto sequence = QKeySequence(event->modifiers() | event->key());
  if(m_selected_quote && sequence == QKeySequence(Qt::CTRL + Qt::Key_K)) {
    if(auto selected_quote = m_selected_quote->get()) {
      on_cancel_most_recent(*selected_quote);
    }
  } else if(m_selected_quote &&
      sequence == QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_K)) {
    if(auto selected_quote = m_selected_quote->get()) {
      on_cancel_all(*selected_quote);
    }
  } else if(auto operation =
      m_key_bindings->get_cancel_key_bindings()->find_operation(sequence)) {
    m_cancel_order_signal(*operation, m_security_view->get_current()->get(),
      none);
  }
}

void BookViewWindow::on_context_menu(MarketDepth* market_depth,
    const QPoint& pos) {
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
  auto properties_window = m_factory->make(m_key_bindings,
    m_security_view->get_current()->get(), m_markets);
  if(!properties_window->isVisible()) {
    properties_window->show();
    if(screen()->geometry().right() - frameGeometry().right() >=
        properties_window->frameGeometry().width()) {
      properties_window->move(frameGeometry().right(), y());
    } else {
      properties_window->move(x() - properties_window->frameGeometry().width(),
        y());
    }
  }
  properties_window->activateWindow();
}

void BookViewWindow::on_current(const Security& security) {
  setWindowTitle(to_text(security) + " " + QString(0x2013) + " " + TITLE_NAME);
  m_transition_view->set_status(TransitionView::Status::NONE);
  auto interaction_key_bindings =
    m_key_bindings->get_interactions_key_bindings(security);
  m_is_cancel_on_fill = interaction_key_bindings->is_cancel_on_fill();
  auto default_quantity = interaction_key_bindings->get_default_quantity();
  m_default_bid_quantity->set(default_quantity->get());
  m_default_ask_quantity->set(default_quantity->get());
  m_model = m_model_builder(security);
  auto body = new QWidget();
  auto layout = make_vbox_layout(body);
  auto panel = new TechnicalsPanel(m_model->get_technicals(),
    m_default_bid_quantity, m_default_ask_quantity);
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
          m_is_cancel_on_fill->get()) {
        m_cancel_order_signal(cancel_operation,
          m_security_view->get_current()->get(), none);
      }
    });
}
