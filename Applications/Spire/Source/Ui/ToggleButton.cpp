#include "Spire/Ui/ToggleButton.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto DEFAULT_SIZE() {
    static auto size = scale(26, 26);
    return size;
  }
}

ToggleButton::ToggleButton(QImage icon, QWidget* parent)
    : QAbstractButton(parent),
      m_icon(std::move(icon)),
      m_icon_button(nullptr) {
  setCheckable(true);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  update_button();
  connect(this, &ToggleButton::toggled, this, &ToggleButton::update_button);
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

void ToggleButton::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    update_button();
  } else if(event->type() == QEvent::ToolTipChange) {
    if(m_icon_button != nullptr) {
      m_icon_button->setToolTip(toolTip());
    }
  }
  QAbstractButton::changeEvent(event);
}

void ToggleButton::paintEvent(QPaintEvent* event) {}

QSize ToggleButton::sizeHint() const {
  return DEFAULT_SIZE();
}

void ToggleButton::update_button() {
  auto is_button_focused = [&] {
    if(m_icon_button != nullptr) {
      return m_icon_button->hasFocus();
    }
    return false;
  }();
  if(m_icon_button != nullptr) {
    m_icon_button->clearFocus();
  }
  delete_later(m_icon_button);
  auto style = [&] {
    auto style = IconButton::Style();
    if(isChecked()) {
      style.m_default_color = "#1FD37A";
      style.m_hover_color = "#2CAC79";
      style.m_blur_color = "#1FD37A";
    } else {
      style.m_default_color = "#7F5EEC";
      style.m_hover_color = "#4B23A0";
      style.m_blur_color = "#7F5EEC";
    }
    return style;
  }();
  m_icon_button = new IconButton(m_icon, style, this);
  setFocusProxy(m_icon_button);
  m_icon_button->setToolTip(toolTip());
  m_icon_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  connect(m_icon_button, &IconButton::clicked, [=] {
    nextCheckState();
    Q_EMIT clicked(isChecked());
  });
  layout()->addWidget(m_icon_button);
  if(is_button_focused) {
    m_icon_button->setFocus();
  }
}
