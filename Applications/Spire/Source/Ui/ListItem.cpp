#include "Spire/Ui/ListItem.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

ListItem::ListItem(QWidget* component, QWidget* parent)
    : QWidget(parent) {
  setFocusPolicy(Qt::StrongFocus);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_button = new Button(component, this);
  setFocusProxy(m_button);
  m_button->installEventFilter(this);
  layout->addWidget(m_button);
  auto style = get_style(*m_button);
  style.get(Any() > Button::Body()).set(
    BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF)));
  style.get(Hover() > Button::Body()).set(
    BackgroundColor(QColor::fromRgb(0xF2, 0xF2, 0xFF)));
  style.get(Focus() > Button::Body()).set(
    BackgroundColor(QColor::fromRgb(0x68, 0x4B, 0xC7)));
  set_style(*m_button, std::move(style));
}

bool ListItem::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::FocusIn) {
    m_current_signal();
  }
  return QWidget::eventFilter(watched, event);
}

connection ListItem::connect_current_signal(
    const CurrentSignal::slot_type& slot) const {
  return m_current_signal.connect(slot);
}

connection ListItem::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_button->connect_clicked_signal(slot);
}
