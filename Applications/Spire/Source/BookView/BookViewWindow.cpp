#include "Spire/BookView/BookViewWindow.hpp"
#include <QScreen>
#include "Spire/BookView/MarketDepth.hpp"
#include "Spire/BookView/TechnicalsPanel.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/SecurityView.hpp"
#include "Spire/Ui/TransitionView.hpp"
#include "Spire/Utilities/LinkMenu.hpp"

using namespace Nexus;
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
  //m_properties_connection = m_factory->get_properties()->connect_update_signal(
  //  std::bind_front(&BookViewWindow::on_properties, this));
  resize(scale(206, 361));
}

void BookViewWindow::on_context_menu(MarketDepth* market_depth, const QPoint& pos) {
  auto menu = new ContextMenu(*market_depth);
  if(auto selected_quote = market_depth->get_selected_book_quote()->get()) {
    menu->add_action(tr("Cancel Most Recent"),
      std::bind_front(&BookViewWindow::on_cancel_most_recent, this, *selected_quote));
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
}

void BookViewWindow::on_cancel_all(const BookQuote& book_quote) {
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
  auto properties_window = m_factory->make(m_key_bindings, security, m_markets);
  auto default_quantity = m_key_bindings->get_interactions_key_bindings(security)->get_default_quantity();
  m_default_bid_quantity->set(default_quantity->get());
  m_default_ask_quantity->set(default_quantity->get());
  auto model = m_model_builder(security);
  auto widget = new QWidget();
  auto layout = make_vbox_layout(widget);
  auto panel = new TechnicalsPanel(model->get_technicals(),
    m_default_bid_quantity, m_default_ask_quantity);
  panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  layout->addWidget(panel);
  if(security == Security()) {
    layout->addStretch(1);
  } else {
    auto market_depth = new MarketDepth(model, model->get_bbo_quote(), m_factory->get_properties(), m_markets);
    market_depth->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(market_depth);
    widget->setFocusProxy(market_depth);
    connect(market_depth, &QWidget::customContextMenuRequested,
      std::bind_front(&BookViewWindow::on_context_menu, this, market_depth));
  }
  m_transition_view->set_body(*widget);
  m_transition_view->set_status(TransitionView::Status::READY);
}
