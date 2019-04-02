#include "spire/book_view/book_view_window.hpp"
#include <QContextMenuEvent>
#include <QEvent>
#include <QMenu>
#include <QTimer>
#include "spire/book_view/book_view_properties_dialog.hpp"
#include "spire/book_view/book_view_table_widget.hpp"
#include "spire/book_view/labeled_data_widget.hpp"
#include "spire/book_view/technicals_panel.hpp"
#include "spire/security_input/security_input_dialog.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/custom_qt_variants.hpp"
#include "spire/ui/drop_shadow.hpp"
#include "spire/ui/security_widget.hpp"
#include "spire/ui/transition_widget.hpp"
#include "spire/ui/window.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

BookViewWindow::BookViewWindow(const BookViewProperties& properties,
    Ref<SecurityInputModel> input_model, QWidget* parent)
    : QWidget(parent),
      m_input_model(input_model.Get()),
      m_is_data_loaded(false),
      m_technicals_panel(nullptr) {
  m_body = new QWidget(this);
  m_body->setMinimumSize(scale(220, 280));
  resize(scale(220, 410));
  m_body->setStyleSheet("background-color: #FFFFFF;");
  auto window_layout = new QVBoxLayout(this);
  window_layout->setContentsMargins({});
  auto window = new Window(m_body, this);
  setWindowTitle(tr("Book View"));
  window->set_svg_icon(":/icons/bookview-black.svg",
    ":/icons/bookview-grey.svg");
  setWindowIcon(QIcon(":icons/book-view-icon-256x256.png"));
  window_layout->addWidget(window);
  m_body->installEventFilter(this);
  auto body_layout = new QVBoxLayout(m_body);
  body_layout->setContentsMargins({});
  body_layout->setSpacing(0);
  m_security_widget = new SecurityWidget(input_model,
    SecurityWidget::Theme::LIGHT, this);
  body_layout->addWidget(m_security_widget);
  m_container_widget = new QWidget(this);
  m_layout = new QVBoxLayout(m_container_widget);
  m_layout->setContentsMargins({});
  m_layout->setSpacing(0);
}

void BookViewWindow::set_model(std::shared_ptr<BookViewModel> model) {
  auto item_delegate = CustomVariantItemDelegate();
  setWindowTitle(item_delegate.displayText(QVariant::fromValue(
    model->get_security()), QLocale()) + tr(" - Book View"));
  if(m_technicals_panel == nullptr) {
    m_technicals_panel = new TechnicalsPanel(this);
    m_layout->addWidget(m_technicals_panel);
    m_quote_widgets_container = new QWidget(this);
    m_quote_widgets_container_layout = new QVBoxLayout(
      m_quote_widgets_container);
    m_quote_widgets_container_layout->setContentsMargins({});
    m_quote_widgets_container_layout->setSpacing(0);
    m_layout->addWidget(m_quote_widgets_container);
    m_security_widget->set_widget(m_container_widget);
  } else {
    m_technicals_panel->reset_model();
  }
  m_bbo_quote_panel.reset();
  m_table.reset();
  if(m_transition_widget == nullptr) {
    m_transition_widget = std::make_unique<TransitionWidget>(
      m_quote_widgets_container);
  }
  m_is_data_loaded = false;
  m_model = std::move(model);
  m_data_loaded_promise = m_model->load();
  m_data_loaded_promise.then(
    [=] (auto&& value) { on_data_loaded(std::move(value)); });
}

const BookViewProperties& BookViewWindow::get_properties() const {
  return m_properties;
}

void BookViewWindow::set_properties(const BookViewProperties& properties) {
  m_properties = properties;
  if(m_table != nullptr) {
    m_table->set_properties(m_properties);
  }
}

connection BookViewWindow::connect_security_change_signal(
    const ChangeSecuritySignal::slot_type& slot) const {
  return m_security_widget->connect_change_security_signal(slot);
}

connection BookViewWindow::connect_closed_signal(
    const ClosedSignal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void BookViewWindow::closeEvent(QCloseEvent* event) {
  m_closed_signal();
}

bool BookViewWindow::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_body) {
    if(event->type() == QEvent::ContextMenu) {
      show_context_menu(static_cast<QContextMenuEvent*>(event)->globalPos());
    }
  }
  return QWidget::eventFilter(watched, event);
}

void BookViewWindow::keyPressEvent(QKeyEvent* event) {
  QApplication::sendEvent(m_security_widget, event);
}

void BookViewWindow::show_context_menu(const QPoint& pos) {
  QMenu context_menu(this);
  QAction properties_action(tr("Properties"), &context_menu);
  connect(&properties_action, &QAction::triggered, this,
    &BookViewWindow::show_properties_dialog);
  context_menu.addAction(&properties_action);
  context_menu.setFixedSize(scale(140, 28));
  context_menu.setWindowFlag(Qt::NoDropShadowWindowHint);
  DropShadow context_menu_shadow(true, true, &context_menu);
  context_menu.setStyleSheet(QString(R"(
    QMenu {
      background-color: #FFFFFF;
      border: %1px solid #A0A0A0 %2px solid #A0A0A0;
      color: #000000;
      font-family: Roboto;
      font-size: %3px;
      padding: %4px 0px;
    }

    QMenu::item {
      padding: %5px 0px %5px %6px;
    }

    QMenu::item:selected, QMenu::item:hover {
      background-color: #8D78EC;
      color: #FFFFFF;
    })")
    .arg(scale_height(1)).arg(scale_width(1))
    .arg(scale_height(12)).arg(scale_height(4))
    .arg(scale_height(2)).arg(scale_width(8)));
  context_menu.exec(pos);
}

void BookViewWindow::show_properties_dialog() {
  BookViewPropertiesDialog dialog(get_properties(), Security(), this);
  dialog.connect_apply_signal([=] (auto p) { set_properties(p); });
  m_security_widget->show_overlay_widget();
  if(dialog.exec() == QDialog::Accepted) {
    set_properties(dialog.get_properties());
  }
  m_security_widget->hide_overlay_widget();
}

void BookViewWindow::on_data_loaded(Expect<void> value) {
  m_transition_widget.reset();
  m_is_data_loaded = true;
  m_technicals_panel->set_model(m_model);
  m_bbo_quote_panel = std::make_unique<BboQuotePanel>(*m_model, this);
  m_quote_widgets_container_layout->addWidget(m_bbo_quote_panel.get());
  m_table = std::make_unique<BookViewTableWidget>(*m_model, m_properties, this);
  m_quote_widgets_container_layout->addWidget(m_table.get());
}
