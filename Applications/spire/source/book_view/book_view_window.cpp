#include "spire/book_view/book_view_window.hpp"
#include <QContextMenuEvent>
#include <QEvent>
#include <QMenu>
#include <QVBoxLayout>
#include "spire/book_view/book_view_properties_dialog.hpp"
#include "spire/book_view/labeled_data_widget.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/drop_shadow.hpp"
#include "spire/ui/window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

book_view_window::book_view_window(const book_view_properties& properties,
    security_input_model& input_model, QWidget* parent)
    : QWidget(parent) {
  m_body = new QWidget(this);
  m_body->setMinimumSize(scale(210, 280));
  resize(scale(232, 410));
  m_body->setStyleSheet("background-color: #FFFFFF;");
  auto window_layout = new QVBoxLayout(this);
  window_layout->setContentsMargins({});
  auto window = new spire::window(m_body, this);
  setWindowTitle(tr("Book View"));
  window->set_svg_icon(":/icons/bookview-black.svg",
    ":/icons/bookview-grey.svg");
  window_layout->addWidget(window);
  m_body->installEventFilter(this);
  auto layout = new QVBoxLayout(m_body);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_header_widget = new QWidget(this);
  m_header_widget->setFixedHeight(scale_height(36));
  m_header_widget->setStyleSheet("background-color: #F5F5F5;");
  m_header_widget->installEventFilter(this);
  layout->addWidget(m_header_widget);
  m_header_layout = new QGridLayout(m_header_widget);
  m_header_layout->setContentsMargins(scale_width(8), 0, scale_width(8), 0);
  m_header_layout->setSpacing(0);
  m_high_label_widget = new labeled_data_widget(tr("H"), tr("0.00"),
    this);
  m_header_layout->addWidget(m_high_label_widget, 0, 0);
  m_header_layout->setColumnStretch(0, 100);
  m_open_label_widget = new labeled_data_widget(tr("O"), tr("0.00"),
    this);
  m_header_layout->addWidget(m_open_label_widget, 0, 1);
  m_header_layout->setColumnStretch(1, 100);
  m_def_label_widget = new labeled_data_widget(tr("D"),
    QString("100%1100").arg(tr("x")), this);
  m_header_layout->addWidget(m_def_label_widget, 0, 2);
  m_header_layout->setColumnStretch(2, 1);
  m_low_label_widget = new labeled_data_widget(tr("L"), tr("0.00"),
    this);
  m_header_layout->addWidget(m_low_label_widget, 1, 0);
  m_close_label_widget = new labeled_data_widget(tr("C"), tr("0.00"),
    this);
  m_header_layout->addWidget(m_close_label_widget, 1, 1);
  m_volume_label_widget = new labeled_data_widget(tr("V"), "0", this);
  m_header_layout->addWidget(m_volume_label_widget, 1, 2);
  m_empty_window_label = new QLabel(tr("Enter a ticker symbol."), this);
  m_empty_window_label->setAlignment(Qt::AlignCenter);
  m_empty_window_label->setStyleSheet(QString(R"(
    font-family: Roboto;
    font-size: %1px;)").arg(scale_height(12)));
  layout->addWidget(m_empty_window_label);
}

void book_view_window::set_model(std::shared_ptr<book_view_model> model) {
}

const book_view_properties& book_view_window::get_properties() const {
  return m_properties;
}

void book_view_window::set_properties(const book_view_properties& properties) {
}

connection book_view_window::connect_security_change_signal(
    const change_security_signal::slot_type& slot) const {
  return m_change_security_signal.connect(slot);
}

connection book_view_window::connect_closed_signal(
    const closed_signal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

bool book_view_window::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_header_widget) {
    if(event->type() == QEvent::Resize) {
      on_header_resize();
    }
  } else if(watched == m_body) {
    if(event->type() == QEvent::ContextMenu) {
      show_context_menu(static_cast<QContextMenuEvent*>(event)->globalPos());
    }
  }
  return QWidget::eventFilter(watched, event);
}

void book_view_window::show_context_menu(const QPoint& pos) {
  QMenu context_menu(this);
  QAction properties_action(tr("Properties"), &context_menu);
  connect(&properties_action, &QAction::triggered, this,
    &book_view_window::show_properties_dialog);
  context_menu.addAction(&properties_action);
  context_menu.setFixedSize(scale(140, 28));
  context_menu.setWindowFlag(Qt::NoDropShadowWindowHint);
  drop_shadow context_menu_shadow(true, true, &context_menu);
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

void book_view_window::show_properties_dialog() {
  book_view_properties_dialog dialog(get_properties(), Security(), this);
  dialog.connect_apply_signal([=] (auto p) { set_properties(p); });
  show_overlay_widget();
  if(dialog.exec() == QDialog::Accepted) {
    set_properties(dialog.get_properties());
  }
  m_overlay_widget.reset();
}

void book_view_window::set_labeled_data_long_form_text() {
  m_high_label_widget->set_label_text(tr("High"));
  m_open_label_widget->set_label_text(tr("Open"));
  m_def_label_widget->set_label_text(tr("Def"));
  m_low_label_widget->set_label_text(tr("Low"));
  m_close_label_widget->set_label_text(tr("Close"));
  m_volume_label_widget->set_label_text(tr("Vol"));
}

void book_view_window::set_labeled_data_short_form_text() {
  m_high_label_widget->set_label_text(tr("H"));
  m_open_label_widget->set_label_text(tr("O"));
  m_def_label_widget->set_label_text(tr("D"));
  m_low_label_widget->set_label_text(tr("L"));
  m_close_label_widget->set_label_text(tr("C"));
  m_volume_label_widget->set_label_text(tr("V"));
}

void book_view_window::show_overlay_widget() {
  m_overlay_widget = std::make_unique<QWidget>(m_body);
  m_overlay_widget->setStyleSheet(
    "background-color: rgba(245, 245, 245, 153);");
  m_overlay_widget->resize(m_body->size());
  m_overlay_widget->move(m_header_widget->pos());
  m_overlay_widget->show();
}

void book_view_window::update_header_layout() {
  if(m_header_widget->width() <= scale_width(412)) {
    if(m_header_layout->itemAtPosition(0, 3) != nullptr) {
      m_header_widget->setFixedHeight(scale_height(36));
      m_header_layout->addWidget(m_low_label_widget, 1, 0);
      m_header_layout->addWidget(m_close_label_widget, 1, 1);
      m_header_layout->addWidget(m_volume_label_widget, 1, 2);
      m_header_layout->setColumnStretch(2, 1);
      m_header_layout->setColumnStretch(3, 0);
      m_header_layout->setColumnStretch(4, 0);
      m_header_layout->setColumnStretch(5, 0);
    }
  } else {
    if(m_header_layout->itemAtPosition(1, 0) != nullptr) {
      m_header_widget->setFixedHeight(scale_height(20));
      m_header_layout->addWidget(m_low_label_widget, 0, 3);
      m_header_layout->addWidget(m_close_label_widget, 0, 4);
      m_header_layout->addWidget(m_volume_label_widget, 0, 5);
      m_header_layout->setColumnStretch(2, 100);
      m_header_layout->setColumnStretch(3, 100);
      m_header_layout->setColumnStretch(4, 100);
      m_header_layout->setColumnStretch(5, 1);
    }
  }
}

void book_view_window::on_header_resize() {
  auto header_width = m_header_widget->width();
  if(header_width <= scale_width(255)) {
    set_labeled_data_short_form_text();
  } else if(header_width >= scale_width(256) &&
      header_width <= scale_width(411)) {
    set_labeled_data_long_form_text();
  } else if(header_width >= scale_width(412) &&
      header_width <= scale_width(503)) {
    set_labeled_data_short_form_text();
  } else if(header_width >= scale_width(504)) {
    set_labeled_data_long_form_text();
  }
  update_header_layout();
}
