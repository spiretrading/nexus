#include "Spire/BookView/BookViewWindow.hpp"
#include <QContextMenuEvent>
#include <QEvent>
#include <QMenu>
#include <QTimer>
#include "Spire/BookView/BookViewPropertiesDialog.hpp"
#include "Spire/BookView/BookViewTableWidget.hpp"
#include "Spire/BookView/LabeledDataWidget.hpp"
#include "Spire/BookView/TechnicalsPanel.hpp"
#include "Spire/SecurityInput/SecurityInputDialog.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropShadow.hpp"
#include "Spire/Ui/SecurityWidget.hpp"
#include "Spire/Ui/TransitionWidget.hpp"
#include "Spire/Ui/Window.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

BookViewWindow::BookViewWindow(const BookViewProperties& properties,
    Ref<SecurityInputModel> input_model, QWidget* parent)
    : Window(parent),
      m_input_model(input_model.Get()),
      m_is_data_loaded(false),
      m_technicals_panel(nullptr) {
  setMinimumSize(scale(220, 280));
  resize_body(scale(220, 410));
  setWindowTitle(tr("Book View"));
  set_svg_icon(":/Icons/bookview-black.svg", ":/Icons/bookview-grey.svg");
  setWindowIcon(QIcon(":/Icons/book-view-icon-256x256.png"));
  m_security_widget = new SecurityWidget(input_model,
    SecurityWidget::Theme::LIGHT, this);
  layout()->addWidget(m_security_widget);
}

void BookViewWindow::set_model(std::shared_ptr<BookViewModel> model) {
  auto item_delegate = CustomVariantItemDelegate();
  setWindowTitle(item_delegate.displayText(QVariant::fromValue(
    model->get_security()), QLocale()) + tr(" - Book View"));
  if(m_technicals_panel == nullptr) {
    auto container_widget = new QWidget(this);
    m_layout = new QVBoxLayout(container_widget);
    m_layout->setContentsMargins({});
    m_layout->setSpacing(0);
    m_technicals_panel = new TechnicalsPanel(this);
    m_layout->addWidget(m_technicals_panel);
    m_quote_widgets_container = new QWidget(this);
    m_quote_widgets_container_layout = new QVBoxLayout(
      m_quote_widgets_container);
    m_quote_widgets_container_layout->setContentsMargins({});
    m_quote_widgets_container_layout->setSpacing(0);
    m_layout->addWidget(m_quote_widgets_container);
    m_security_widget->set_widget(container_widget);
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

void BookViewWindow::contextMenuEvent(QContextMenuEvent* event) {
  show_context_menu(event->globalPos());
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
  auto context_menu_shadow = new DropShadow(true, true, &context_menu);
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
  m_dialog_apply_connection = dialog.connect_apply_signal(
    [=, &dialog] { set_properties(dialog.get_properties()); });
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
