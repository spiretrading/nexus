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
      m_is_data_loaded(false) {
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
  m_layout = new QVBoxLayout(m_body);
  m_layout->setContentsMargins({});
  m_layout->setSpacing(0);
  m_empty_window_label = std::make_unique<QLabel>(
    tr("Enter a ticker symbol."), this);
  m_empty_window_label->setAlignment(Qt::AlignCenter);
  m_empty_window_label->setStyleSheet(QString(R"(
    font-family: Roboto;
    font-size: %1px;)").arg(scale_height(12)));
  m_layout->addWidget(m_empty_window_label.get());
}

void BookViewWindow::set_model(std::shared_ptr<BookViewModel> model) {
  m_model = std::move(model);
  m_technicals_panel->reset_model();
  QTimer::singleShot(2000, this, [=] { show_transition_widget(); });
  m_is_data_loaded = false;
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
  return m_change_security_signal.connect(slot);
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
  if(event->key() == Qt::Key_PageUp) {
    if(m_current_security != Security()) {
      auto s = m_securities.push_front(m_current_security);
      if(s != Security()) {
        set_current(s);
      }
    }
    return;
  } else if(event->key() == Qt::Key_PageDown) {
    if(m_current_security != Security()) {
      auto s = m_securities.push_back(m_current_security);
      if(s != Security()) {
        set_current(s);
      }
    }
    return;
  }
  auto pressed_key = event->text();
  if(pressed_key[0].isLetterOrNumber()) {
    auto dialog = new SecurityInputDialog(Ref(*m_input_model), pressed_key,
      this);
    dialog->setWindowModality(Qt::NonModal);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &QDialog::accepted, this,
      [=] { on_security_input_accept(dialog); });
    connect(dialog, &QDialog::rejected, this,
      [=] { on_security_input_reject(dialog); });
    dialog->move(geometry().center().x() -
      dialog->width() / 2, geometry().center().y() - dialog->height() / 2);
    show_overlay_widget();
    dialog->show();
  }
}

void BookViewWindow::set_current(const Security& s) {
  if(s == m_current_security) {
    return;
  }
  if(m_empty_window_label != nullptr) {
    m_empty_window_label.reset();
    m_technicals_panel = new TechnicalsPanel(this);
    m_layout->addWidget(m_technicals_panel);
    m_quote_widgets_container = new QWidget(this);
    m_quote_widgets_container_layout = new QVBoxLayout(
      m_quote_widgets_container);
    m_quote_widgets_container_layout->setContentsMargins({});
    m_quote_widgets_container_layout->setSpacing(0);
    m_layout->addWidget(m_quote_widgets_container);
  }
  m_bbo_quote_panel.reset();
  m_table.reset();
  m_current_security = s;
  m_change_security_signal(s);
  CustomVariantItemDelegate item_delegate;
  setWindowTitle(item_delegate.displayText(QVariant::fromValue(s), QLocale())
    + tr(" - Book View"));
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

void BookViewWindow::show_overlay_widget() {
  m_overlay_widget = std::make_unique<QWidget>(m_body);
  m_overlay_widget->setStyleSheet(
    "background-color: rgba(245, 245, 245, 153);");
  m_overlay_widget->resize(m_body->size());
  m_overlay_widget->move(0, 0);
  m_overlay_widget->show();
}

void BookViewWindow::show_properties_dialog() {
  BookViewPropertiesDialog dialog(get_properties(), Security(), this);
  dialog.connect_apply_signal([=] (auto p) { set_properties(p); });
  show_overlay_widget();
  if(dialog.exec() == QDialog::Accepted) {
    set_properties(dialog.get_properties());
  }
  m_overlay_widget.reset();
}

void BookViewWindow::show_transition_widget() {
  if(!m_is_data_loaded && m_transition_widget == nullptr) {
    m_transition_widget = std::make_unique<TransitionWidget>(
      m_quote_widgets_container);
  }
}

void BookViewWindow::on_security_input_accept(
    SecurityInputDialog* dialog) {
  auto s = dialog->get_security();
  if(s != Security() && s != m_current_security) {
    m_securities.push(m_current_security);
    set_current(s);
    activateWindow();
  }
  dialog->close();
  m_overlay_widget.reset();
}

void BookViewWindow::on_security_input_reject(
    SecurityInputDialog* dialog) {
  dialog->close();
  m_overlay_widget.reset();
}

void BookViewWindow::on_data_loaded(Expect<void> value) {
  m_transition_widget.reset();
  m_is_data_loaded = true;
  m_technicals_panel->set_model(m_model);
  m_bbo_quote_panel = std::make_unique<BboQuotePanel>(*m_model, this);
  m_quote_widgets_container_layout->addWidget(m_bbo_quote_panel.get());
  m_table = std::make_unique<BookViewTableWidget>(this);
  m_quote_widgets_container_layout->addWidget(m_table.get());
  m_bbo_quote_panel->set_model(*m_model);
  m_table->set_model(m_model);
  m_table->set_properties(m_properties);
}
