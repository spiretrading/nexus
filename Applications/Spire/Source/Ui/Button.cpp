#include "Spire/Ui/Button.hpp"
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Tooltip.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

Button::Button(QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_body(body),
      m_press_observer(*this),
      m_click_observer(*this) {
  setFocusPolicy(Qt::StrongFocus);
  match(*m_body, Body());
  enclose(*this, *m_body);
  m_press_observer.connect_press_start_signal(
    std::bind_front(&Button::on_press_start, this));
  m_press_observer.connect_press_end_signal(
    std::bind_front(&Button::on_press_end, this));
  m_click_observer.connect_click_signal(m_clicked_signal);
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

void Button::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      if(!event->isAutoRepeat()) {
        m_clicked_signal();
      }
      break;
    default:
      QWidget::keyPressEvent(event);
  }
}

void Button::on_press_start(PressObserver::Reason reason) {
  match(*this, Press());
}

void Button::on_press_end(PressObserver::Reason reason) {
  unmatch(*this, Press());
}

Button* Spire::make_icon_button(QImage icon, QWidget* parent) {
  return make_icon_button(icon, "", parent);
}

Button* Spire::make_icon_button(
    QImage icon, QString tooltip_text, QWidget* parent) {
  auto button_icon = new Icon(icon);
  button_icon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto button = new Button(new Box(button_icon), parent);
  auto tooltip = new Tooltip(tooltip_text, button);
  auto style = StyleSheet();
  style.get(Any() > Body()).
    set(BackgroundColor(QColor(Qt::transparent))).
    set(border(scale_width(1), QColor(Qt::transparent)));
  style.get((Hover() || Press()) > Body()).
    set(BackgroundColor(QColor(0xE0E0E0)));
  style.get(FocusVisible() > Body()).set(border_color(QColor(0x4B23A0)));
  style.get(Any() > is_a<Icon>()).set(Fill(QColor(0x535353)));
  style.get(Hover() > is_a<Icon>()).set(Fill(QColor(0x4B23A0)));
  style.get(Press() > is_a<Icon>()).set(Fill(QColor(0x7E71B8)));
  style.get(Disabled() > is_a<Icon>()).set(Fill(QColor(0xC8C8C8)));
  set_style(*button, std::move(style));
  return button;
}

Button* Spire::make_delete_icon_button(QWidget* parent) {
  auto button = make_icon_button(
    imageFromSvg(":/Icons/delete.svg", scale(16, 16)), parent);
  update_style(*button, [&] (auto& style) {
    style.get(Any() > is_a<Box>()).
      set(BackgroundColor(QColor(Qt::transparent)));
    style.get(Any() > is_a<Icon>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(Fill(QColor(0xA0A0A0)));
    style.get((Hover() || Press()) > Body() > is_a<Icon>()).
      set(BackgroundColor(QColor(0xEBEBEB))).
      set(Fill(QColor(0x4B23A0)));
    style.get(Disabled() > Body() > is_a<Icon>()).set(Fill(QColor(0xD0D0D0)));
  });
  return button;
}

Button* Spire::make_label_button(const QString& label, QWidget* parent) {
  auto label_box = make_label(label);
  auto button = new Button(label_box, parent);
  button->setFixedHeight(scale_height(26));
  auto style = StyleSheet();
  style.get(Any() > Body()).
    set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
    set(border(scale_width(1), QColor(Qt::transparent))).
    set(BackgroundColor(QColor(0xEBEBEB))).
    set(horizontal_padding(scale_width(8)));
  style.get(Hover() > Body()).
    set(TextColor(QColor(0xFFFFFF))).
    set(BackgroundColor(QColor(0x4B23A0)));
  style.get(Press() > Body()).
    set(TextColor(QColor(0xFFFFFF))).
    set(BackgroundColor(QColor(0x7E71B8)));
  style.get(FocusVisible() > Body()).set(border_color(QColor(0x4B23A0)));
  style.get(Disabled() > Body()).set(TextColor(QColor(0xB8B8B8)));
  set_style(*button, std::move(style));
  return button;
}
