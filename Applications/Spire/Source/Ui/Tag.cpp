#include "Spire/Ui/Tag.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor(0xEBEBEB))).
      set(border_radius(scale_width(3))).
      set(border_size(0));
    style.get(Any() >> is_a<TextBox>()).
      set(border_size(0)).
      set(horizontal_padding(scale_width(5))).
      set(vertical_padding(scale_height(2)));
    style.get(ReadOnly() >> is_a<Button>()).
      set(Visibility::NONE);
    return style;
  }

  auto DELETE_BUTTON_STYLE(StyleSheet style) {
    style.get((Hover() || Press()) / Body() >> is_a<Icon>()).
      set(BackgroundColor(QColor(Qt::transparent)));
    return style;
  }
}

Tag::Tag(QString label, QWidget* parent)
    : QWidget(parent),
      m_is_read_only(false) {
  auto container = new QWidget(this);
  auto container_layout = make_hbox_layout(container);
  auto label_box = make_label(std::move(label));
  label_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  container_layout->addWidget(label_box);
  m_delete_button = make_delete_icon_button();
  m_delete_button->setFixedSize(scale(16, 16));
  m_delete_button->setFocusPolicy(Qt::NoFocus);
  update_style(*m_delete_button, [&] (auto& style) {
    style = DELETE_BUTTON_STYLE(style);
  });
  container_layout->addWidget(m_delete_button);
  auto box = new Box(container);
  enclose(*this, *box);
  proxy_style(*this, *box);
  set_style(*this, DEFAULT_STYLE());
}

bool Tag::is_read_only() const {
  return m_is_read_only;
}

void Tag::set_read_only(bool is_read_only) {
  m_is_read_only = is_read_only;
  if(m_is_read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
}

connection Tag::connect_delete_signal(
    const DeleteSignal::slot_type& slot) const {
  return m_delete_button->connect_clicked_signal(slot);
}
