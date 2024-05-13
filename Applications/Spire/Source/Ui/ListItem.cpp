#include "Spire/Ui/ListItem.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace boost;
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
    : ListItem(parent) {
  mount(body);
}

ListItem::ListItem(QWidget* parent)
    : QWidget(parent),
      m_is_current(false),
      m_is_selected(false) {
  auto layout = make_hbox_layout(this);
  layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
  setFocusPolicy(Qt::ClickFocus);
  set_style(*this, DEFAULT_STYLE());
}

bool ListItem::is_current() const {
  return m_is_current;
}

void ListItem::set_current(bool is_current) {
  m_is_current = is_current;
  if(m_is_current) {
    match(*this, Current());
  } else {
    unmatch(*this, Current());
  }
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

bool ListItem::is_mounted() const {
  return m_box.has_value();
}

QWidget& ListItem::get_body() {
  if(m_box) {
    return *m_box->get_body();
  }
  return *this;
}

connection ListItem::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

QSize ListItem::sizeHint() const {
  if(m_box) {
    return m_box->sizeHint();
  }
  return QWidget::sizeHint();
}

void ListItem::mount(QSpacerItem& body) {
  if(auto item = layout()->takeAt(0)) {
    delete item;
  }
  static_cast<QBoxLayout&>(*layout()).addSpacerItem(&body);
}

void ListItem::mount(QWidget& body) {
  m_box.emplace(&body, Box::Fit::BOTH);
  m_button.emplace(&*m_box, this);
  m_button->layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);
  m_button->setFocusPolicy(Qt::ClickFocus);
  m_button->connect_click_signal(m_submit_signal);
  if(body.isEnabled()) {
    setFocusProxy(&body);
  } else {
    m_box->setFocusProxy(nullptr);
    setFocusProxy(&*m_button);
  }
  if(auto item = layout()->takeAt(0)) {
    delete item;
  }
  layout()->addWidget(&*m_button);
  link(*this, body);
  match(body, Body());
  proxy_style(*m_button, *m_box);
  proxy_style(*this, *m_button);
  updateGeometry();
}

QWidget* ListItem::unmount() {
  auto size_hint = sizeHint();
  auto size_policy = get_body().sizePolicy();
  auto item = layout()->takeAt(0);
  delete item;
  auto widget = [&] () -> QWidget* {
    if(!m_box) {
      return nullptr;
    }
    auto body = m_box->get_body();
    body->hide();
    body->setParent(nullptr);
    return body;
  }();
  m_box = none;
  m_button = none;
  static_cast<QBoxLayout&>(*layout()).addSpacerItem(
    new QSpacerItem(size_hint.width(), size_hint.height(),
      size_policy.horizontalPolicy(), size_policy.verticalPolicy()));
  updateGeometry();
  return widget;
}
