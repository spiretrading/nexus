#include "Spire/Ui/ToggleButton.hpp"
#include <QEvent>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace boost::signals2;
using namespace Spire;

ToggleButton::ToggleButton(QImage icon, QWidget* parent)
    : QWidget(parent),
      m_icon(std::move(icon)),
      m_icon_button(nullptr),
      m_is_toggled(false) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  update_button();
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
  QWidget::setEnabled(enabled);
  update_button(enabled);
}

void ToggleButton::setDisabled(bool disabled) {
  QWidget::setDisabled(disabled);
  update_button(!disabled);
}

connection ToggleButton::connect_clicked_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

void ToggleButton::swap_toggle() {
  m_is_toggled = !m_is_toggled;
  update_button();
}

void ToggleButton::update_button() {
  delete_later(m_icon_button);
  auto style = IconButton::Style();
  if(m_is_toggled) {
    style.m_default_color = "#1FD37A";
    style.m_hover_color = "#1FD37A";
    style.m_blur_color = "#1FD37A";
  } else {
    style.m_default_color = "#7F5EEC";
    style.m_hover_color = "#4B23A0";
    style.m_blur_color = "#7F5EEC";
  }
  m_icon_button = new IconButton(m_icon, style, this);
  m_clicked_connection = m_icon_button->connect_clicked_signal([=] {
    swap_toggle();
    m_clicked_signal();
  });
  setFocusProxy(m_icon_button);
  m_icon_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout()->addWidget(m_icon_button);
}

void ToggleButton::update_button(bool enabled) {
  if(enabled) {
    update_button();
  }
}
