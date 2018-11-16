#include "spire/charting/charting_window.hpp"
#include <climits>
#include <QHBoxLayout>
#include <QIcon>
#include <QIntValidator>
#include <QKeyEvent>
#include <QListView>
#include <QVBoxLayout>
#include "spire/security_input/security_input_dialog.hpp"
#include "spire/security_input/security_input_model.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/custom_qt_variants.hpp"
#include "spire/ui/dropdown_menu.hpp"
#include "spire/ui/toggle_button.hpp"
#include "spire/ui/window.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

ChartingWindow::ChartingWindow(Ref<SecurityInputModel> input_model,
    QWidget* parent)
    : QWidget(parent),
      m_input_model(input_model.Get()) {
  m_body = new QWidget(this);
  m_body->installEventFilter(this);
  m_body->setMinimumSize(scale(400, 320));
  m_body->resize(scale(400, 320));
  m_body->setStyleSheet("background-color: #FFFFFF;");
  auto window_layout = new QHBoxLayout(this);
  window_layout->setContentsMargins({});
  auto window = new Window(m_body, this);
  setWindowTitle(tr("Chart"));
  window->set_svg_icon(":/icons/chart-black.svg",
    ":/icons/chart-grey.svg");
  setWindowIcon(QIcon(":/icons/chart-icon-256x256.png"));
  window_layout->addWidget(window);
  auto layout = new QVBoxLayout(m_body);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_button_header_widget = new QWidget(m_body);
  m_button_header_widget->setFixedHeight(scale_height(46));
  m_button_header_widget->setStyleSheet("background-color: #F5F5F5;");
  auto button_header_layout = new QHBoxLayout(m_button_header_widget);
  button_header_layout->setSpacing(0);
  button_header_layout->setContentsMargins(0, scale_height(10), 0,
    scale_height(10));
  button_header_layout->addSpacing(scale_width(8));
  m_period_line_edit = new QLineEdit(m_button_header_widget);
  connect(m_period_line_edit, &QLineEdit::editingFinished, this,
    &ChartingWindow::on_period_line_edit_changed);
  m_period_line_edit->setFixedSize(scale(36, 26));
  m_period_line_edit->setValidator(new QIntValidator(1, INT_MAX,
    m_period_line_edit));
  m_period_line_edit->setText("1");
  m_period_line_edit->setAlignment(Qt::AlignCenter);
  m_period_line_edit->setStyleSheet(QString(R"(
    QLineEdit {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8 %2px solid #C8C8C8;
      color: #000000;
      font-family: Roboto;
      font-size: %3px;
    }

    QLineEdit:focus {
      border: %1px solid #4B23A0 %2px solid #4B23A0;
    })").arg(scale_height(1)).arg(scale_width(1)).arg(scale_height(12)));
  button_header_layout->addWidget(m_period_line_edit);
  button_header_layout->addSpacing(scale_width(4));
  m_period_dropdown = new DropdownMenu(
    {tr("second"), tr("minute"), tr("hour")}, m_button_header_widget);
  m_period_dropdown->setFixedSize(scale(80, 26));
  m_period_dropdown->installEventFilter(this);
  button_header_layout->addWidget(m_period_dropdown);
  button_header_layout->addSpacing(scale_width(18));
  auto button_image_size = scale(16, 16);
  auto lock_grid_button = new ToggleButton(
    imageFromSvg(":/icons/lock-grid-purple.svg", button_image_size),
    imageFromSvg(":/icons/lock-grid-green.svg", button_image_size),
    imageFromSvg(":/icons/lock-grid-purple.svg", button_image_size),
    imageFromSvg(":/icons/lock-grid-grey.svg", button_image_size),
    m_button_header_widget);
  lock_grid_button->setFixedSize(scale(26, 26));
  button_header_layout->addWidget(lock_grid_button);
  button_header_layout->addSpacing(scale_width(10));
  auto auto_scale_button = new ToggleButton(
    imageFromSvg(":/icons/auto-scale-purple.svg", button_image_size),
    imageFromSvg(":/icons/auto-scale-green.svg", button_image_size),
    imageFromSvg(":/icons/auto-scale-purple.svg", button_image_size),
    imageFromSvg(":/icons/auto-scale-grey.svg", button_image_size),
    m_button_header_widget);
  auto_scale_button->setFixedSize(scale(26, 26));
  button_header_layout->addWidget(auto_scale_button);
  button_header_layout->addSpacing(scale_width(10));
  auto seperator = new QWidget(m_button_header_widget);
  seperator->setFixedSize(scale(1, 16));
  seperator->setStyleSheet("background-color: #D0D0D0;");
  button_header_layout->addWidget(seperator);
  button_header_layout->addSpacing(scale_width(10));
  auto draw_line_button = new ToggleButton(
    imageFromSvg(":/icons/draw-purple.svg", button_image_size),
    imageFromSvg(":/icons/draw-green.svg", button_image_size),
    imageFromSvg(":/icons/draw-purple.svg", button_image_size),
    imageFromSvg(":/icons/draw-grey.svg", button_image_size),
    m_button_header_widget);
  draw_line_button->setFixedSize(scale(26, 26));
  button_header_layout->addWidget(draw_line_button);
  button_header_layout->addStretch(1);
  layout->addWidget(m_button_header_widget);
  m_empty_window_label = std::make_unique<QLabel>(tr("Enter a ticker symbol."),
    this);
  m_empty_window_label->setAlignment(Qt::AlignCenter);
  m_empty_window_label->setStyleSheet(QString(R"(
    background-color: #25212E;
    color: #FFFFFF;
    font-family: Roboto;
    font-size: %1px;
    padding-top: %2px;)").arg(scale_height(12)).arg(scale_height(16)));
  layout->addWidget(m_empty_window_label.get());
  setTabOrder(m_period_line_edit, m_period_dropdown);
  setTabOrder(m_period_dropdown, lock_grid_button);
  setTabOrder(lock_grid_button, auto_scale_button);
  setTabOrder(auto_scale_button, draw_line_button);
}

bool ChartingWindow::eventFilter(QObject* object, QEvent* event) {
  if(object == m_period_dropdown) {
    if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->text()[0].isLetterOrNumber()) {
        show_security_input_dialog(e->text());
      }
    }
  } else if(object == m_body) {
    if(event->type() == QEvent::MouseButtonPress) {
      m_body->setFocus();
    }
  }
  return false;
}

void ChartingWindow::keyPressEvent(QKeyEvent* event) {
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
    show_security_input_dialog(pressed_key);
  }
}

void ChartingWindow::set_current(const Security& s) {
  if(s == m_current_security) {
    return;
  }
  m_current_security = s;
  m_change_security_signal(s);
  setWindowTitle(
    CustomVariantItemDelegate().displayText(QVariant::fromValue(s),
      QLocale()) + tr(" - Chart"));
}

void ChartingWindow::show_overlay_widget() {
  auto contents = m_body->layout()->itemAt(1)->widget();
  m_overlay_widget = std::make_unique<QLabel>(m_body);
  m_overlay_widget->setStyleSheet(
    "background-color: rgba(245, 245, 245, 153);");
  m_overlay_widget->resize(contents->size());
  m_overlay_widget->move(contents->mapTo(contents, contents->pos()));
  m_overlay_widget->show();
}

void ChartingWindow::show_security_input_dialog(const QString& text) {
  auto dialog = new SecurityInputDialog(Ref(*m_input_model), text,
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

void ChartingWindow::on_period_line_edit_changed() {
  auto value = m_period_line_edit->text().toInt();
  if(value == 1) {
    m_period_dropdown->set_items({tr("second"), tr("minute"), tr("hour")});
  } else {
    m_period_dropdown->set_items({tr("seconds"), tr("minutes"), tr("hours")});
  }
}

void ChartingWindow::on_security_input_accept(SecurityInputDialog* dialog) {
  auto s = dialog->get_security();
  if(s != Security() && s != m_current_security) {
    m_securities.push(m_current_security);
    set_current(s);
    activateWindow();
  }
  dialog->close();
  m_overlay_widget.reset();
}

void ChartingWindow::on_security_input_reject(SecurityInputDialog* dialog) {
  dialog->close();
  m_overlay_widget.reset();
}
