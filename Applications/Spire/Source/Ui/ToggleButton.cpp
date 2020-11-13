#include "Spire/Ui/ToggleButton.hpp"
#include <QEvent>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

ToggleButton::ToggleButton(QImage icon, QImage toggle_icon, QWidget* parent)
    : ToggleButton(icon, toggle_icon, icon, parent) {}

ToggleButton::ToggleButton(QImage icon, QImage toggle_icon, QImage hover_icon,
    QWidget* parent)
    : ToggleButton(icon, toggle_icon, hover_icon, icon, parent) {}

ToggleButton::ToggleButton(QImage icon, QImage toggle_icon, QImage hover_icon,
    QImage disable_icon, QWidget* parent)
    : QWidget(parent),
      m_is_toggled(false),
      m_icon_button(new IconButton(icon, parent)),
      m_icon(std::move(icon)),
      m_hover_icon(std::move(hover_icon)),
      m_toggle_icon(std::move(toggle_icon)),
      m_disabled_icon(std::move(disable_icon)) {
  setFocusProxy(m_icon_button);
  m_icon_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_clicked_connection = m_icon_button->connect_clicked_signal([=] {
    swap_toggle();
  });
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_icon_button);
  setStyleSheet(QString(R"(
    QToolTip {
      background-color: white;
      border: 1px solid #C8C8C8;
      color: black;
      font-family: Roboto;
      font-size: %1px;
      padding: %2px %3px %2px %3px;
    })").arg(scale_height(10)).arg(scale_height(2)).arg(scale_width(6)));
}

void ToggleButton::set_toggled(bool toggled) {
  if(m_is_toggled != toggled) {
    swap_toggle();
  }
}

void ToggleButton::setEnabled(bool enabled) {
  update_icons(enabled);
  QWidget::setEnabled(enabled);
}

void ToggleButton::setDisabled(bool disabled) {
  update_icons(!disabled);
  QWidget::setDisabled(disabled);
}

connection ToggleButton::connect_clicked_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_icon_button->connect_clicked_signal(slot);
}

void ToggleButton::swap_toggle() {
  m_is_toggled = !m_is_toggled;
  update_icons();
}

void ToggleButton::update_icons() {
  if(m_is_toggled) {
    //m_icon_button->set_icon(m_toggle_icon, m_toggle_icon);
  } else {
    //m_icon_button->set_icon(m_icon, m_hover_icon);
  }
}

void ToggleButton::update_icons(bool enabled) {
  if(enabled) {
    update_icons();
  }
}
