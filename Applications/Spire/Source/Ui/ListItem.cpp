#include "Spire/Ui/ListItem.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border(scale_width(1), QColor(Qt::transparent))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(5)));
    style.get(Hover()).set(BackgroundColor(QColor(0xF2F2FF)));
    style.get(Current()).set(BackgroundColor(QColor(0xD0D0D0)));
    style.get(Selected()).set(BackgroundColor(QColor(0xE2E0FF)));
    style.get(Disabled()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border_color(QColor(Qt::transparent)));
    return style;
  }
}

ListItem::ListItem(QWidget& body, QWidget* parent)
    : QWidget(parent),
      m_is_selected(false) {
  setFocusPolicy(Qt::ClickFocus);
  m_box = new Box(&body, Box::Fit::BOTH);
  m_button = new Button(m_box, this);
  m_button->layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);
  m_button->setFocusPolicy(Qt::ClickFocus);
  if(body.isEnabled()) {
    setFocusProxy(&body);
  } else {
    m_box->setFocusProxy(nullptr);
    setFocusProxy(m_button);
  }
  auto layout = make_hbox_layout(this);
  layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
  layout->addWidget(m_button);
  proxy_style(*m_button, *m_box);
  proxy_style(*this, *m_button);
  set_style(*this, DEFAULT_STYLE());
}

bool ListItem::is_selected() const {
  return m_is_selected;
}

void ListItem::set_selected(bool is_selected) {
  m_is_selected = is_selected;
  if(m_is_selected) {
    match(*this, Selected());
  } else {
    unmatch(*this, Selected());
  }
}

QWidget& ListItem::get_body() {
  return *m_box->get_body();
}

connection ListItem::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_button->connect_click_signal(slot);
}
