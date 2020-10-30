#include "Spire/Ui/TransitionWidget.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMovie>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

TransitionWidget::TransitionWidget(QWidget* parent)
    : QWidget(parent),
      m_show_timer(this) {
  setStyleSheet("background-color: transparent;");
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  auto backing_widget = new QLabel(this);
  auto logo = new QMovie(":/Icons/pre-loader.gif", QByteArray(),
    backing_widget);
  logo->setScaledSize(scale(32, 32));
  backing_widget->setMovie(logo);
  backing_widget->setStyleSheet(
    QString("padding-top: %1px;").arg(scale_height(50)));
  backing_widget->setAlignment(Qt::AlignHCenter);
  backing_widget->movie()->start();
  layout->addWidget(backing_widget);
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
