#include "spire/ui/overlay_widget.hpp"
#include <QEvent>

using namespace spire;

overlay_widget::overlay_widget(QWidget* covered, QWidget* displayed,
    QWidget* parent)
    : QWidget(parent),
      m_covered(covered),
      m_displayed(displayed) {
  align_displayed();
  m_covered->raise();
  m_covered->installEventFilter(this);
}

bool overlay_widget::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_covered) {
    if(event->type() == QEvent::Move || event->type() == QEvent::Resize) {
      align_displayed();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void overlay_widget::align_displayed() {
  m_displayed->resize(m_covered->size());
  m_displayed->move(m_covered->mapTo(m_covered, m_covered->pos()));
}
