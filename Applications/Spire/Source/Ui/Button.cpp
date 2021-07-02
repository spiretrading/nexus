#include "Spire/Ui/Button.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/TextBox.hpp"

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
  unmatch(*this, Hover());
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

void Button::mouseMoveEvent(QMouseEvent* event) {
  if(!rect().contains(event->pos())) {
    unmatch(*this, Hover());
  } else if(m_is_down) {
    match(*this, Hover());
  }
  QWidget::mouseMoveEvent(event);
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
      match(*this, Hover());
      if(m_is_down) {
        m_is_down = false;
        m_clicked_signal();
      }
    }
  }
  QWidget::mouseReleaseEvent(event);
}

Button* Spire::make_icon_button(QImage icon, QWidget* parent) {
  auto button = new Button(new Icon(icon, parent), parent);
  auto style = StyleSheet();
  style.get(Body()).
    set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
    set(border(scale_width(1), QColor::fromRgb(0, 0, 0, 0)));
  style.get(Hover() / Body()).set(
    BackgroundColor(QColor::fromRgb(0xE3, 0xE3, 0xE3)));
  style.get(Focus() / Body()).set(
    border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
  set_style(*button, std::move(style));
  return button;
}

Button* Spire::make_label_button(const QString& label, QWidget* parent) {
  auto text_box = new TextBox(label);
  text_box->set_read_only(true);
  text_box->setDisabled(true);
  auto button = new Button(text_box, parent);
  auto style = StyleSheet();
  style.get(Body()).
    set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
    set(BackgroundColor(QColor::fromRgb(0xEB, 0xEB, 0xEB))).
    set(TextColor(QColor::fromRgb(0, 0, 0))).
    set(border(scale_width(1), QColor::fromRgb(0, 0, 0, 0))).
    set(horizontal_padding(scale_width(8)));
  style.get(Hover() / Body()).
    set(BackgroundColor(QColor::fromRgb(0x4B, 0x23, 0xA0))).
    set(TextColor(QColor::fromRgb(0xFF, 0xFF, 0xFF)));
  style.get(Press() / Body()).
    set(TextColor(QColor::fromRgb(0xFF, 0xFF, 0xFF)));
  style.get(Focus() / Body()).set(
    border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
  style.get(Disabled() / Body()).
    set(BackgroundColor(QColor::fromRgb(0xEB, 0xEB, 0xEB))).
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
