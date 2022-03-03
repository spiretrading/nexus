#include "Spire/Ui/ToggleButton.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/Tooltip.hpp"

using namespace Spire;
using namespace Spire::Styles;

ToggleButton::ToggleButton(QWidget* body, QWidget* parent)
  : ToggleButton(body, std::make_shared<LocalBooleanModel>()) {}

ToggleButton::ToggleButton(QWidget* body, std::shared_ptr<BooleanModel> current,
    QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)) {
  setFocusPolicy(Qt::StrongFocus);
  m_button = new Button(body);
  setFocusProxy(m_button);
  enclose(*this, *m_button);
  find_stylist(*m_button).connect_match_signal(Press(),
    [=] (auto is_match) {
      if(is_match) {
        match(*this, Press());
      } else {
        unmatch(*this, Press());
      }
    });
  proxy_style(*this, *m_button);
  m_button->connect_click_signal(
    std::bind_front(&ToggleButton::on_click, this));
  if(m_current->get()) {
    match(*this, Checked());
  }
  m_connection = m_current->connect_update_signal(
    std::bind_front(&ToggleButton::on_update, this));
}

const QWidget& ToggleButton::get_body() const {
  return m_button->get_body();
}

QWidget& ToggleButton::get_body() {
  return m_button->get_body();
}

const std::shared_ptr<BooleanModel>& ToggleButton::get_current() const {
  return m_current;
}

void ToggleButton::on_click() {
  m_current->set(!m_current->get());
}

void ToggleButton::on_update(bool current) {
  if(current) {
    match(*this, Checked());
  } else {
    unmatch(*this, Checked());
  }
}

ToggleButton* Spire::make_icon_toggle_button(QImage icon, QWidget* parent) {
  return make_icon_toggle_button(std::move(icon), "", parent);
}

ToggleButton* Spire::make_icon_toggle_button(
    QImage icon, QString tooltip, QWidget* parent) {
  auto button_icon = new Icon(std::move(icon));
  button_icon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto button = new ToggleButton(new Box(button_icon), parent);
  if(!tooltip.isEmpty()) {
    new Tooltip(std::move(tooltip), button);
  }
  auto style = StyleSheet();
  style.get(Any() > Body()).
    set(BackgroundColor(QColor(Qt::transparent))).
    set(border(scale_width(1), QColor(Qt::transparent)));
  style.get((Hover() || Press()) > Body()).
    set(BackgroundColor(QColor(0xE0E0E0)));
  style.get(Any() > (is_a<Button>() && FocusVisible()) > Body()).
    set(border_color(QColor(0x4B23A0)));
  style.get(Any() > is_a<Icon>()).set(Fill(QColor(0x535353)));
  style.get(Hover() > is_a<Icon>()).set(Fill(QColor(0x4B23A0)));
  style.get(Press() > is_a<Icon>()).set(Fill(QColor(0x7E71B8)));
  style.get(Disabled() > is_a<Icon>()).set(Fill(QColor(0xC8C8C8)));
  style.get((Checked() && !Hover() && !Press()) > is_a<Icon>()).
    set(Fill(QColor(0x7F5EEC)));
  style.get((Checked() && Disabled()) > is_a<Icon>()).
    set(Fill(QColor(0xBAB3D9)));
  set_style(*button, std::move(style));
  return button;
}
