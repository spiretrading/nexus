#include "Spire/Ui/TransitionWidget.hpp"
#include <QEvent>
#include <QLabel>
#include <QMovie>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;

TransitionWidget::TransitionWidget(QWidget* parent)
    : QWidget(parent),
      m_show_timer(this) {
  setObjectName("transition_widget");
  setStyleSheet("#transition_widget { background-color: transparent; }");
  auto backing_widget = new QLabel();
  auto logo = new QMovie(":/Icons/pre-loader.gif", QByteArray());
  logo->setScaledSize(scale(32, 32));
  backing_widget->setMovie(logo);
  backing_widget->setStyleSheet(
    QString("QLabel { padding-top: %1px; }").arg(scale_height(50)));
  backing_widget->setAlignment(Qt::AlignHCenter);
  backing_widget->movie()->start();
  enclose(*this, *backing_widget);
  parent->installEventFilter(this);
  hide();
  connect(&m_show_timer, &QTimer::timeout, [=] { on_show_timer(); });
  m_show_timer.setSingleShot(true);
  m_show_timer.start(2000);
}

bool TransitionWidget::eventFilter(QObject* watched, QEvent* event) {
  if(watched == parent()) {
    if(event->type() == QEvent::Move || event->type() == QEvent::Resize) {
      align();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void TransitionWidget::align() {
  auto parent_widget = static_cast<QWidget*>(parent());
  resize(parent_widget->size());
  move(parent_widget->mapTo(parent_widget, parent_widget->pos()));
}

void TransitionWidget::on_show_timer() {
  align();
  show();
}
