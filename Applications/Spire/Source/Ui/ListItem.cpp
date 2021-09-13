#include "Spire/Ui/ListItem.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"

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
    style.get(Hover()).set(
      BackgroundColor(QColor(0xF2F2FF)));
    style.get(Focus()).set(
      border_color(QColor(0x4B23A0)));
    style.get(Selected()).set(
      BackgroundColor(QColor(0xE2E0FF)));
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
    m_current_signal();
  }
  return QWidget::eventFilter(watched, event);
}
