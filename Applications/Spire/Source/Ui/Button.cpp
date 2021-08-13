#include "Spire/Ui/Button.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Tooltip.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

Button::Button(QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_body(body),
      m_is_down(false) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_body);
  setFocusPolicy(Qt::StrongFocus);
}

const QWidget& Button::get_body() const {
  return *m_body;
}

QWidget& Button::get_body() {
  return *m_body;
}

connection Button::connect_clicked_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

void Button::focusOutEvent(QFocusEvent* event) {
  if(event->reason() != Qt::PopupFocusReason && m_is_down) {
    m_is_down = false;
  }
  unmatch(*this, Press());
  QWidget::focusOutEvent(event);
}

void Button::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      if(!event->isAutoRepeat()) {
        m_clicked_signal();
      }
      break;
    case Qt::Key_Space:
      if(!event->isAutoRepeat()) {
        m_is_down = true;
        match(*this, Press());
      }
      break;
    default:
      QWidget::keyPressEvent(event);
  }
}

void Button::keyReleaseEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Space:
      if(!event->isAutoRepeat() && m_is_down) {
        m_is_down = false;
        unmatch(*this, Press());
        m_clicked_signal();
      }
      break;
    default:
      QWidget::keyPressEvent(event);
  }
}

void Button::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton && rect().contains(event->pos())) {
    m_is_down = true;
    match(*this, Press());
  }
  QWidget::mousePressEvent(event);
}

void Button::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    unmatch(*this, Press());
    if(rect().contains(event->pos())) {
      if(m_is_down) {
        m_is_down = false;
        m_clicked_signal();
      }
    }
  }
  QWidget::mouseReleaseEvent(event);
}

Button* Spire::make_icon_button(QImage icon, QWidget* parent) {
  return make_icon_button(icon, "", parent);
}

Button* Spire::make_icon_button(QImage icon, QString tooltip_text,
    QWidget* parent) {
  auto button_icon = new Icon(icon, parent);
  button_icon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto button = new Button(new Box(button_icon), parent);
  auto tooltip = new Tooltip(tooltip_text, button);
  auto style = StyleSheet();
  style.get(Body()).
    set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
    set(border(scale_width(1), QColor::fromRgb(0, 0, 0, 0)));
  style.get((Hover() || Press()) / Body()).
    set(BackgroundColor(QColor::fromRgb(0xE0, 0xE0, 0xE0)));
  style.get(Focus() / Body()).
    set(border_color(QColor(0x4B, 0x23, 0xA0)));
  style.get(Any() >> is_a<Icon>()).
    set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
    set(Fill(QColor::fromRgb(0x53, 0x53, 0x53)));
  style.get((Hover() || Press()) / Body() >> is_a<Icon>()).
    set(Fill(QColor(0x4B, 0x23, 0xA0)));
  style.get(Disabled() / Body() >> is_a<Icon>()).
    set(Fill(QColor(0xD0, 0xD0, 0xD0)));
  set_style(*button, std::move(style));
  return button;
}

Button* Spire::make_delete_icon_button(QWidget* parent) {
  auto button = make_icon_button(imageFromSvg(":/Icons/delete.svg",
    scale(16, 16)), parent);
  auto style = get_style(*button);
  style.get(Any() >> is_a<Box>()).
    set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0)));
  style.get(Any() >> is_a<Icon>()).
    set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
    set(Fill(QColor::fromRgb(0xA0, 0xA0, 0xA0)));
  style.get((Hover() || Press()) / Body() >> is_a<Icon>()).
    set(BackgroundColor(QColor::fromRgb(0xEB, 0xEB, 0xEB))).
    set(Fill(QColor(0x4B, 0x23, 0xA0)));
  style.get(Disabled() / Body() >> is_a<Icon>()).
    set(Fill(QColor(0xD0, 0xD0, 0xD0)));
  set_style(*button, std::move(style));
  return button;
}

Button* Spire::make_label_button(const QString& label, QWidget* parent) {
  auto label_box = make_label(label);
  auto button = new Button(label_box, parent);
  auto style = StyleSheet();
  style.get(Body()).
    set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
    set(border(scale_width(1), QColor::fromRgb(0, 0, 0, 0))).
    set(BackgroundColor(QColor::fromRgb(0xEB, 0xEB, 0xEB))).
    set(horizontal_padding(scale_width(8)));
  style.get((Hover() || Press()) / Body()).
    set(TextColor(QColor::fromRgb(0xFF, 0xFF, 0xFF))).
    set(BackgroundColor(QColor::fromRgb(0x4B, 0x23, 0xA0)));
  style.get(Focus() / Body()).set(
    border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
  style.get(Disabled() / Body()).
    set(TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
  set_style(*button, std::move(style));
  return button;
}

std::unordered_set<Stylist*> BaseComponentFinder<Button, Body>::operator ()(
    Button& button, const Body& body) const {
  auto stylists = std::unordered_set<Stylist*>();
  stylists.insert(&find_stylist(button.get_body()));
  return stylists;
}
