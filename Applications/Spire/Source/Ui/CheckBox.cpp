#include "Spire/Ui/Checkbox.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
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
    style.get(Any() >> is_a<Icon>()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(Fill(QColor::fromRgb(0x33, 0x33, 0x33)));
    style.get(!Checked() >> is_a<Icon>()).
      set(Fill(QColor(0, 0, 0, 0)));
    style.get((Disabled() && Checked()) >> is_a<Icon>()).
      set(Fill(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    style.get(Any() >> is_a<Box>()).
      set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF))).
      set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    style.get((Focus() || Hover()) >> is_a<Box>()).
      set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    style.get(Disabled() >> is_a<Box>()).
      set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
      set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    style.get(ReadOnly() >> is_a<Box>()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0)));
    style.get(Any() >> is_a<TextBox>()).
      set(padding(0));
    style.get((Disabled()) >> is_a<TextBox>()).
      set(TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }
}

CheckBox::CheckBox(QWidget* parent)
  : CheckBox(std::make_shared<LocalBooleanModel>(false), parent) {}

CheckBox::CheckBox(std::shared_ptr<BooleanModel> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_is_read_only(false) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  auto body = new QWidget(this);
  auto button = new Button(body, this);
  setFocusProxy(button);
  button->connect_clicked_signal([=] {
    m_model->set_current(!m_model->get_current());
  });
  layout->addWidget(button);
  auto body_layout = new QHBoxLayout(body);
  body_layout->setContentsMargins({});
  body_layout->setSpacing(0);
  auto check = new Icon(CHECK_ICON(), parent);
  check->setFocusPolicy(Qt::NoFocus);
  auto check_box = new Box(check, this);
  check_box->setFixedSize(scale(16, 16));
  body_layout->addWidget(check_box);
  m_label = make_label("", this);
  body_layout->addWidget(m_label);
  m_model->connect_current_signal([=] (auto is_checked) {
    on_checked(is_checked);
  });
  on_checked(m_model->get_current());
  set_style(*this, DEFAULT_STYLE());
  on_layout_direction(layoutDirection());
}

void CheckBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::LayoutDirectionChange) {
    on_layout_direction(layoutDirection());
  }
}

const std::shared_ptr<BooleanModel>& CheckBox::get_model() const {
  return m_model;
}

void CheckBox::set_label(const QString& label) {
  m_label->get_model()->set_current(label);
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

QSize CheckBox::sizeHint() const {
  return scale(80, 16);
}

void CheckBox::on_checked(bool is_checked) {
  if(is_checked) {
    match(*this, Checked());
  } else {
    unmatch(*this, Checked());
  }
}

void CheckBox::on_layout_direction(Qt::LayoutDirection direction) {
  auto style = get_style(*this);
  auto [padding_left, padding_right] = [&] {
    if(direction == Qt::LeftToRight) {
      return std::make_tuple(scale_width(8), 0);
    }
    return std::make_tuple(0, scale_width(8));
  }();
  style.get(Any() >> is_a<TextBox>()).
    set(PaddingLeft(padding_left)).
    set(PaddingRight(padding_right));
  set_style(*this, style);
}

CheckBox* Spire::make_radio_button(QWidget* parent) {
  auto button = new CheckBox(parent);
  auto style = get_style(*button);
  style.get(Any() >> is_a<Icon>()).
    set(IconImage(RADIO_CHECK_ICON()));
  style.get(Any() >> is_a<Box>()).
    set(border_radius(scale_width(8)));
  set_style(*button, std::move(style));
  return button;
}
