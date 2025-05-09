#include "Spire/Ui/Tag.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"

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
    style.get(Any() > is_a<TextBox>()).
      set(border_size(0)).
      set(horizontal_padding(scale_width(5))).
      set(vertical_padding(scale_height(2)));
    style.get(ReadOnly() > is_a<Button>()).
      set(Visibility::NONE);
    return style;
  }

  auto DELETE_BUTTON_STYLE(StyleSheet style) {
    style.get((Hover() || Press()) > is_a<Icon>()).
      set(BackgroundColor(QColor(Qt::transparent)));
    return style;
  }
}

Tag::Tag(QString label, QWidget* parent)
  : Tag(std::make_shared<LocalTextModel>(std::move(label)), parent) {}

Tag::Tag(std::shared_ptr<TextModel> label, QWidget* parent)
    : QWidget(parent),
      m_is_read_only(false) {
  auto container = new QWidget();
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
  link(*this, *label_box);
  link(*this, *m_delete_button);
  proxy_style(*this, *box);
  set_style(*this, DEFAULT_STYLE());
}

const std::shared_ptr<TextModel>& Tag::get_label() const {
  auto box = static_cast<Box*>(layout()->itemAt(0)->widget());
  auto label_box =
    static_cast<TextBox*>(box->get_body()->layout()->itemAt(0)->widget());
  return label_box->get_current();
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
  return m_delete_button->connect_click_signal(slot);
}
