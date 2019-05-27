#include "spire/ui/dropdown_color_picker.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/color_picker.hpp"
#include "spire/ui/flat_button.hpp"

using namespace Spire;

DropdownColorPicker::DropdownColorPicker(QWidget* parent)
    : QWidget(parent) {
  window()->installEventFilter(this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_button = new FlatButton(this);
  auto button_default_style = m_button->get_style();
  button_default_style.m_background_color = QColor("#FFCA19");
  button_default_style.m_border_color = QColor("#C8C8C8");
  m_button->set_style(button_default_style);
  auto button_hover_style = m_button->get_hover_style();
  button_hover_style.m_border_color = QColor("4B23A0");
  button_hover_style.m_background_color =
    button_default_style.m_background_color;
  m_button->set_hover_style(button_hover_style);
  m_button->connect_clicked_signal([=] { on_button_click(); });
  layout->addWidget(m_button);
  setFocusProxy(m_button);

  m_color_picker = new ColorPicker(scale_width(70) - 4, scale_height(70) - 3,
    this);
  m_color_picker->setFixedSize(scale(70, 70));
  m_color_picker->hide();
}

bool DropdownColorPicker::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Move) {
    if(m_color_picker->isVisible()) {
      move_color_picker();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DropdownColorPicker::resizeEvent(QResizeEvent* event) {
  m_button->resize(size());
}

void DropdownColorPicker::showEvent(QShowEvent* event) {
  move_color_picker();
}

void DropdownColorPicker::move_color_picker() {
  auto pos = mapToGlobal(m_button->geometry().bottomLeft());
  m_color_picker->move(pos.x(), pos.y() + scale_height(1));
  m_color_picker->raise();
}

void DropdownColorPicker::on_button_click() {
  if(m_color_picker->isVisible()) {
    m_color_picker->hide();
  } else {
    m_color_picker->show();
  }
}
