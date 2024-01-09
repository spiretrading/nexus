#include "Spire/Ui/EyeDropperButton.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/EyeDropper.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ToggleButton.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Styles;

EyeDropperButton::EyeDropperButton(QWidget* parent)
    : QWidget(parent),
      m_current(std::make_shared<LocalColorModel>()) {
  m_button = make_icon_toggle_button(
    imageFromSvg(":/Icons/eye-dropper.svg", scale(16, 16)));
  m_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*m_button, [] (auto& style) {
    style.get((Hover() || Press()) > Body()).
      set(BackgroundColor(QColor(Qt::transparent)));
    style.get(Any() > (is_a<Button>() && FocusVisible()) > Body()).
      set(border_color(QColor(Qt::transparent)));
    style.get(Hover() > is_a<Icon>()).set(Fill(QColor(0x7F5EEC)));
    style.get(Press() > is_a<Icon>()).set(Fill(QColor(0x7F5EEC)));
    style.get(Disabled() > is_a<Icon>()).set(Fill(QColor(0xC8C8C8)));
  });
  enclose(*this, *m_button);
  proxy_style(*this, *m_button);
  setFocusProxy(m_button);
  m_button->get_current()->connect_update_signal(
    std::bind_front(&EyeDropperButton::on_checked, this));
}

const std::shared_ptr<ValueModel<QColor>>&
    EyeDropperButton::get_current() const {
  return m_current;
}

connection EyeDropperButton::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

connection EyeDropperButton::connect_reject_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_reject_signal.connect(slot);
}

void EyeDropperButton::on_checked(bool checked) {
  if(checked) {
    auto eye_dropper = new EyeDropper(this);
    eye_dropper->get_current()->connect_update_signal(
      std::bind_front(&EyeDropperButton::on_current, this));
    eye_dropper->connect_submit_signal(
      std::bind_front(&EyeDropperButton::on_submit, this));
    eye_dropper->connect_reject_signal(
      std::bind_front(&EyeDropperButton::on_reject, this));
    eye_dropper->show();
  }
}

void EyeDropperButton::on_current(const QColor& current) {
  m_current->set(current);
}

void EyeDropperButton::on_submit(const QColor& submission) {
  m_button->get_current()->set(false);
  m_submit_signal(submission);
}

void EyeDropperButton::on_reject(const QColor& color) {
  m_button->get_current()->set(false);
  m_reject_signal(color);
}
