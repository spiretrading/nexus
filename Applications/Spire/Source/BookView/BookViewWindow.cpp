#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/BookView/MarketDepth.hpp"
#include "Spire/BookView/TechnicalsPanel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/SecurityView.hpp"
#include "Spire/Ui/TransitionView.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto TITLE_NAME = QObject::tr("Book View");
}

BookViewWindow::BookViewWindow(
    std::shared_ptr<SecurityInfoQueryModel> securities,
    std::shared_ptr<KeyBindingsModel> key_bindings,
    std::shared_ptr<BookViewPropertiesWindowFactory> factory,
    MarketDatabase markets, ModelBuilder model_builder, QWidget* parent)
    : Window(parent),
      m_key_bindings(std::move(key_bindings)),
      m_factory(std::move(factory)),
      m_model_builder(std::move(model_builder)),
      m_markets(std::move(markets)),
      m_default_bid_quantity(std::make_shared<LocalQuantityModel>()),
      m_default_ask_quantity(std::make_shared<LocalQuantityModel>()) {
  set_svg_icon(":/Icons/time-sales.svg");
  setWindowIcon(QIcon(":/Icons/taskbar_icons/time-sales.png"));
  setWindowTitle(TITLE_NAME);
  m_transition_view = new TransitionView(new QWidget());
  m_security_view = new SecurityView(std::move(securities), *m_transition_view);
  m_security_view->get_current()->connect_update_signal(
    std::bind_front(&BookViewWindow::on_current, this));
  m_security_view->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_security_view->setContextMenuPolicy(Qt::CustomContextMenu);
  set_body(m_security_view);
  update_style(*this, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  //connect(m_security_view, &QWidget::customContextMenuRequested,
  //  std::bind_front(&BookViewWindow::on_context_menu, this,
  //    m_security_view));
  //m_properties_connection = m_factory->get_properties()->connect_update_signal(
  //  std::bind_front(&BookViewWindow::on_properties, this));
  resize(scale(266, 361));

}

void BookViewWindow::on_current(const Security& security) {
  setWindowTitle(to_text(security) + " " + QString(0x2013) + " " + TITLE_NAME);
  m_transition_view->set_status(TransitionView::Status::NONE);
  auto properties_window = m_factory->make(security);
  auto default_quantity = m_key_bindings->get_interactions_key_bindings(security)->get_default_quantity();
  m_default_bid_quantity->set(default_quantity->get());
  m_default_ask_quantity->set(default_quantity->get());
  auto model = m_model_builder(security);
  auto widget = new QWidget();
  auto layout = make_vbox_layout(widget);
  auto panel = new TechnicalsPanel(model->get_technicals(), m_default_bid_quantity,
    m_default_ask_quantity);
  panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  layout->addWidget(panel);
  if(security == Security()) {
    layout->addStretch(1);
  } else {
    auto market_depth = new MarketDepth(model, model->get_bbo_quote(), m_factory->get_properties(), m_markets);
    layout->addWidget(market_depth);
    widget->setFocusProxy(market_depth);
  }
  m_transition_view->set_body(*widget);
  m_transition_view->set_status(TransitionView::Status::READY);
}

//void BookViewWindow::on_properties(const TimeAndSalesProperties& properties) {
//}
