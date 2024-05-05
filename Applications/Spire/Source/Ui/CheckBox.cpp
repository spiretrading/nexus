#include "Spire/Ui/Checkbox.hpp"
#include <QEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto& CHECK_ICON() {
    static auto icon = imageFromSvg(":/Icons/check.svg", scale(16, 16));
    return icon;
  }

  const auto& RADIO_CHECK_ICON() {
    static auto icon = imageFromSvg(":/Icons/radio-check.svg", scale(16, 16));
    return icon;
  }

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any() > is_a<Icon>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(Fill(QColor(0x333333)));
    style.get(!Checked() > is_a<Icon>()).set(Fill(QColor(0, 0, 0, 0)));
    style.get((Disabled() && Checked()) > is_a<Icon>()).
      set(Fill(QColor(0xC8C8C8)));
    style.get(Any() > is_a<Box>()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border(scale_width(1), QColor(0xC8C8C8)));
    style.get((Focus() || Hover()) > is_a<Box>()).
      set(border_color(QColor(0x4B23A0)));
    style.get(Disabled() > is_a<Box>()).
      set(BackgroundColor(QColor(0xF5F5F5))).
      set(border_color(QColor(0xC8C8C8)));
    style.get(ReadOnly() > is_a<Box>()).
      set(BackgroundColor(QColor(Qt::transparent)));
    style.get(Any() > is_a<TextBox>()).
      set(padding(0));
    style.get((Disabled()) > is_a<TextBox>()).
      set(TextColor(QColor(0xC8C8C8)));
    return style;
  }
}

CheckBox::CheckBox(QWidget* parent)
  : CheckBox(std::make_shared<LocalBooleanModel>(false), parent) {}

CheckBox::CheckBox(std::shared_ptr<BooleanModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_is_read_only(false) {
  auto body = new QWidget(this);
  auto button = new Button(body, this);
  setFocusProxy(button);
  button->connect_click_signal([=] {
    m_current->set(!m_current->get());
  });
  enclose(*this, *button);
  auto check = new Icon(CHECK_ICON(), parent);
  check->setFocusPolicy(Qt::NoFocus);
  auto check_box = new Box(check);
  check_box->setFixedSize(scale(16, 16));
  auto body_layout = make_hbox_layout(body);
  body_layout->addWidget(check_box);
  m_label = make_label("", this);
  body_layout->addWidget(m_label);
  m_connection = m_current->connect_update_signal(
    std::bind_front(&CheckBox::on_current, this));
  link(*this, *check_box);
  link(*this, *check);
  link(*this, *m_label);
  on_current(m_current->get());
  set_style(*this, DEFAULT_STYLE());
  on_layout_direction(layoutDirection());
}

void CheckBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::LayoutDirectionChange) {
    on_layout_direction(layoutDirection());
  }
}

const std::shared_ptr<BooleanModel>& CheckBox::get_current() const {
  return m_current;
}

void CheckBox::set_label(const QString& label) {
  m_label->get_current()->set(label);
}

void CheckBox::set_read_only(bool is_read_only) {
  if(is_read_only != m_is_read_only) {
    m_is_read_only = is_read_only;
    if(m_is_read_only) {
      setAttribute(Qt::WA_TransparentForMouseEvents);
      setFocusPolicy(Qt::NoFocus);
      match(*this, ReadOnly());
    } else {
      setAttribute(Qt::WA_TransparentForMouseEvents, false);
      setFocusPolicy(Qt::StrongFocus);
      unmatch(*this, ReadOnly());
    }
  }
}

void CheckBox::on_current(bool current) {
  if(current) {
    match(*this, Checked());
  } else {
    unmatch(*this, Checked());
  }
}

void CheckBox::on_layout_direction(Qt::LayoutDirection direction) {
  update_style(*this, [&] (auto& style) {
    auto [padding_left, padding_right] = [&] {
      if(direction == Qt::LeftToRight) {
        return std::tuple(scale_width(8), 0);
      }
      return std::tuple(0, scale_width(8));
    }();
    style.get(Any() > is_a<TextBox>()).
      set(PaddingLeft(padding_left)).
      set(PaddingRight(padding_right));
  });
}

CheckBox* Spire::make_radio_button(QWidget* parent) {
  auto button = new CheckBox(parent);
  update_style(*button, [&] (auto& style) {
    style.get(Any() > is_a<Icon>()).set(IconImage(RADIO_CHECK_ICON()));
    style.get(Any() > is_a<Box>()).set(border_radius(scale_width(8)));
  });
  return button;
}
