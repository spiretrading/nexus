#include "Spire/Ui/ColorSelectorButton.hpp"
#include <QEvent>
#include <QKeyEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/DropDownWindow.hpp"

using namespace boost::signals2;
using namespace Spire;

ColorSelectorButton::ColorSelectorButton(const QColor& current_color,
    QWidget* parent)
    : QWidget(parent) {
  setFocusPolicy(Qt::StrongFocus);
  setAttribute(Qt::WA_Hover);
  m_selector_widget = new ColorSelectorDropDown(current_color, this);
  auto dropdown = new DropDownWindow(true, this);
  dropdown->initialize_widget(m_selector_widget);
  m_change_connection = m_selector_widget->connect_changed_signal(
    [=] (const auto& color) {
      on_color_selected(color);
    });
  m_selected_connection = m_selector_widget->connect_selected_signal(
    [=] (const auto& color) {
      on_color_selected(color);
      dropdown->close();
    });
  m_selector_widget->installEventFilter(this);
  set_color(current_color);
}

const QColor& ColorSelectorButton::get_color() const {
  return m_current_color;
}

void ColorSelectorButton::set_color(const QColor& color) {
  m_current_color = color;
  m_selector_widget->set_color(color);
  update();
}

connection ColorSelectorButton::connect_color_signal(
    const ColorSignal::slot_type& slot) const {
  return m_color_signal.connect(slot);
}

bool ColorSelectorButton::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_selector_widget) {
    if(event->type() == QEvent::Show) {
      m_selector_widget->activateWindow();
      m_selector_widget->setFocus();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void ColorSelectorButton::paintEvent(QPaintEvent* event) {
  QWidget::paintEvent(event);
  auto painter = QPainter(this);
  if(hasFocus() || m_selector_widget->isActiveWindow() || underMouse()) {
    painter.fillRect(event->rect(), QColor("#4B23A0"));
  } else {
    painter.fillRect(event->rect(), QColor("#C8C8C8"));
  }
  painter.setPen(Qt::white);
  painter.drawRect(1, 1, width() - 3, height() - 3);
  painter.fillRect(2, 2, width() - 4, height() - 4, m_current_color);
}

QSize	ColorSelectorButton::sizeHint() const {
  return scale(100, 26);
}

void ColorSelectorButton::on_color_selected(const QColor& color) {
  m_current_color = color;
  m_color_signal(m_current_color);
  update();
}
