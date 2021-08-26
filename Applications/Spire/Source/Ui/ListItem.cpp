#include "Spire/Ui/ListItem.hpp"
#include <QFocusEvent>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF))).
      set(border(scale_width(1), QColor::fromRgb(0, 0, 0, 0))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(5)));
    style.get(Hover()).
      set(BackgroundColor(QColor::fromRgb(0xF2, 0xF2, 0xFF)));
    style.get(Focus()).
      set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    style.get(Selected()).
      set(BackgroundColor(QColor::fromRgb(0xE2, 0xE0, 0xFF)));
    style.get(Disabled()).
      set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF))).
      set(border_color(QColor::fromRgb(0, 0, 0, 0)));
    return style;
  }
}

ListItem::ListItem(QWidget* component, QWidget* parent)
    : QWidget(parent),
      m_is_selected(false) {
  setFocusPolicy(Qt::StrongFocus);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_box = new Box(component);
  m_button = new Button(m_box, this);
  if(component->isEnabled()) {
    setFocusProxy(component);
  } else {
    m_box->setFocusProxy(nullptr);
    setFocusProxy(m_button);
  }
  m_button->installEventFilter(this);
  layout->addWidget(m_button);
  set_style(*m_box, DEFAULT_STYLE());
}

bool ListItem::is_selected() const {
  return m_is_selected;
}

void ListItem::set_selected(bool is_selected) {
  m_is_selected = is_selected;
  if(m_is_selected) {
    match(*m_box, Selected());
  } else {
    unmatch(*m_box, Selected());
  }
}

connection ListItem::connect_current_signal(
    const CurrentSignal::slot_type& slot) const {
  return m_current_signal.connect(slot);
}

connection ListItem::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_button->connect_clicked_signal(slot);
}

bool ListItem::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::FocusIn) {
    auto focus_event = static_cast<QFocusEvent*>(event);
    if(focus_event->reason() != Qt::MouseFocusReason) {
      match(*m_box, Focus());
    }
    m_current_signal();
  } else if(event->type() == QEvent::FocusOut) {
    unmatch(*m_box, Focus());
  }
  return QWidget::eventFilter(watched, event);
}
