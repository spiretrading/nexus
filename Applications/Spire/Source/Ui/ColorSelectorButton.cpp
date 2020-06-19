#include "Spire/Ui/ColorSelectorButton.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>

using namespace boost::signals2;
using namespace Spire;

ColorSelectorButton::ColorSelectorButton(const QColor& current_color,
    const RecentColors& recent_colors, QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_button = new FlatButton(this);
  m_button->connect_clicked_signal([=] { on_button_clicked(); });
  m_button->installEventFilter(this);
  layout->addWidget(m_button);
  m_dropdown = new ColorSelectorDropDown(current_color, recent_colors, this);
  m_dropdown->connect_color_signal([=] (const auto& color) {
    on_color_selected(color);
  });
  m_dropdown->hide();
  window()->installEventFilter(this);
  set_color(current_color);
}

void ColorSelectorButton::set_color(const QColor& color) {
  auto style = m_button->get_style();
  style.m_background_color = color;
  style.m_border_color = QColor("#C8C8C8");
  m_button->set_style(style);
  style.m_border_color = QColor("#4B23A0");
  m_button->set_hover_style(style);
  m_button->set_focus_style(style);
  m_dropdown->set_color(color);
}

connection ColorSelectorButton::connect_color_signal(
    const ColorSignal::slot_type& slot) const {
  return m_color_signal.connect(slot);
}

bool ColorSelectorButton::eventFilter(QObject* watched, QEvent* event) {
  if(watched == window()) {
    if(event->type() == QEvent::Move) {
      if(m_dropdown->isVisible()) {
        move_color_dropdown();
      }
    } else if(event->type() == QEvent::WindowDeactivate &&
        !m_dropdown->isActiveWindow()) {
      hide_dropdown();
    } else if(event->type() == QEvent::MouseButtonPress) {
      hide_dropdown();
    }
  } else if(watched == m_button) {
    if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Escape) {
        if(m_dropdown->isVisible()) {
          hide_dropdown();
        }
      }
    } else if(event->type() == QEvent::FocusOut &&
        !m_dropdown->isActiveWindow()) {
      hide_dropdown();
    }
  } else if(watched == m_dropdown) {
    if(event->type() == QEvent::Hide) {
      hide_dropdown();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void ColorSelectorButton::hide_dropdown() {
  m_dropdown->hide();
}

void ColorSelectorButton::move_color_dropdown() {
  auto x_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    geometry().bottomLeft()).x();
  auto y_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    frameGeometry().bottomLeft()).y();
  m_dropdown->move(x_pos, y_pos + 1);
  m_dropdown->raise();
}

void ColorSelectorButton::on_button_clicked() {
  if(m_dropdown->isVisible()) {
    hide_dropdown();
  } else {
    move_color_dropdown();
    m_dropdown->show();
  }
}

void ColorSelectorButton::on_color_selected(const QColor& color) {
  set_color(color);
  m_color_signal(color);
}
