#include "spire/ui/overlay_widget.hpp"
#include <QEvent>
#include <QHBoxLayout>

using namespace Spire;

OverlayWidget::OverlayWidget(QWidget* covered, QWidget* displayed,
    QWidget* parent)
    : QWidget(parent),
      m_covered(covered),
      m_displayed(displayed) {
  setStyleSheet("background-color: transparent;");
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_displayed);
  m_covered->installEventFilter(this);
  align();
  show();
}

bool OverlayWidget::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_covered) {
    if(event->type() == QEvent::Move || event->type() == QEvent::Resize) {
      align();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void OverlayWidget::align() {
  resize(m_covered->size());
  move(m_covered->mapTo(m_covered, m_covered->pos()));
}
