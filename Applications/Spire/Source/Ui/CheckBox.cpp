#include "Spire/Ui/Checkbox.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/LocalValueModel.hpp"
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

  auto DEFAULT_STYLE(Qt::LayoutDirection direction) {
    auto style = StyleSheet();
    auto alignment = [=] {
      if(direction == Qt::LeftToRight) {
        return Qt::AlignRight;
      }
      return Qt::AlignLeft;
    }();
    style.get(Any() >> is_a<Icon>()).
      set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF))).
      set(Fill(QColor::fromRgb(0x33, 0x33, 0x33))).
      set(border(scale_width(1), QColor::fromRgb(0xC8C8C8)));
    style.get((Focus() || (Hover() && !Disabled())) >> is_a<Icon>())
      .set(border_color(QColor(0x4B, 0x23, 0xA0)));
    style.get(Disabled() >> is_a<Icon>()).
      set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5)));
    style.get(ReadOnly() >> is_a<Icon>()).
      set(BackgroundColor(QColor(0, 0, 0, 0))).
      set(border_color(QColor(0, 0, 0, 0)));
    style.get(!Checked() >> is_a<Icon>()).
      set(Fill(QColor(0, 0, 0, 0)));
    style.get((Disabled() && Checked()) >> is_a<Icon>()).
      set(Fill(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    style.get(Any() >> is_a<TextBox>()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(TextColor(QColor(0, 0, 0))).
      set(TextAlign(alignment | Qt::AlignVCenter));
    style.get(Disabled() >> is_a<TextBox>()).
      set(BackgroundColor(QColor(0, 0, 0, 0)));
    style.get(ReadOnly() >> is_a<TextBox>()).
      set(BackgroundColor(QColor(0, 0, 0, 0)));
    style.get((ReadOnly() && !Checked()) >> is_a<TextBox>()).
      set(TextColor(QColor(0, 0, 0, 0)));
    return style;
  }
}

CheckBox::CheckBox(QWidget* parent)
  : CheckBox(std::make_shared<LocalBooleanModel>(false), parent) {}

CheckBox::CheckBox(std::shared_ptr<BoolModel> model, QWidget* parent)
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
  check->setFixedSize(scale(16, 16));
  check->setFocusPolicy(Qt::NoFocus);
  body_layout->addWidget(check);
  m_label = new TextBox(this);
  m_label->set_read_only(true);
  m_label->setDisabled(true);
  body_layout->addWidget(m_label);
  m_model->connect_current_signal([=] (auto is_checked) {
    on_checked(is_checked);
  });
  on_checked(m_model->get_current());
  set_style(*this, DEFAULT_STYLE(layoutDirection()));
}

void CheckBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::LayoutDirectionChange) {
    set_style(*this, DEFAULT_STYLE(layoutDirection()));
  }
}

const std::shared_ptr<BoolModel>& CheckBox::get_model() const {
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
      find_stylist(*this).match(ReadOnly());
    } else {
      setAttribute(Qt::WA_TransparentForMouseEvents, false);
      setFocusPolicy(Qt::StrongFocus);
      find_stylist(*this).unmatch(ReadOnly());
    }
    update();
  }
}

QSize CheckBox::sizeHint() const {
  return scale(80, 16);
}

connection CheckBox::connect_checked_signal(
    const CheckedSignal::slot_type& slot) const {
  return m_checked_signal.connect(slot);
}

void CheckBox::on_checked(bool is_checked) {
  auto& stylist = find_stylist(*this);
  if(is_checked) {
    stylist.match(Checked());
  } else {
    stylist.unmatch(Checked());
  }
  m_checked_signal(is_checked);
}
