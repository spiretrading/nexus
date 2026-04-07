#include "Spire/InputWidgets/TimeInputWidget.hpp"
#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QTimeEdit>
#include <QVBoxLayout>
#include "Spire/InputWidgets/TimeInputDialog.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

TimeInputWidget::TimeInputWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget{parent, flags},
      m_isReadOnly{false} {
  auto layout = new QVBoxLayout{this};
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);
  m_lineEdit = new QLineEdit{this};
  m_lineEdit->installEventFilter(this);
  layout->addWidget(m_lineEdit);
}

TimeInputWidget::TimeInputWidget(Ref<UserProfile> userProfile,
    QWidget* parent, Qt::WindowFlags flags)
    : TimeInputWidget{parent, flags} {
  Initialize(Ref(userProfile));
}

TimeInputWidget::~TimeInputWidget() {}

void TimeInputWidget::Initialize(Ref<UserProfile> userProfile) {
  m_userProfile = userProfile.get();
}

const time_duration& TimeInputWidget::GetTime() const {
  return m_time;
}

void TimeInputWidget::SetTime(time_duration time) {
  m_time = std::move(time);
  QTime timeDisplay{0, 0, 0, 0};
  timeDisplay = timeDisplay.addMSecs(
    static_cast<int>(m_time.total_milliseconds()));
  m_lineEdit->setText(timeDisplay.toString("hh:mm:ss.zzz"));
  m_timeUpdatedSignal(m_time);
}

void TimeInputWidget::SetReadOnly(bool value) {
  m_isReadOnly = value;
}

connection TimeInputWidget::ConnectTimeUpdatedSignal(
    const TimeUpdatedSignal::slot_type& slot) const {
  return m_timeUpdatedSignal.connect(slot);
}

bool TimeInputWidget::eventFilter(QObject* receiver, QEvent* event) {
  if(receiver == m_lineEdit) {
    if(event->type() == QEvent::MouseButtonDblClick) {
      mouseDoubleClickEvent(static_cast<QMouseEvent*>(event));
      return true;
    } else if(event->type() == QEvent::KeyPress) {
      keyPressEvent(static_cast<QKeyEvent*>(event));
      return true;
    }
  }
  return QWidget::eventFilter(receiver, event);
}

void TimeInputWidget::keyPressEvent(QKeyEvent* event) {
  auto text = event->text().trimmed();
  if(event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab ||
      event->key() == Qt::Key_Escape || text.isEmpty()) {
    QWidget::keyPressEvent(event);
    return;
  }
  if(m_isReadOnly) {
    return;
  }
  TimeInputDialog dialog{Ref(*m_userProfile), this};
  QApplication::sendEvent(&dialog, event);
  if(dialog.exec() == QDialog::Rejected) {
    return;
  }
  auto newValue = dialog.GetTime();
  SetTime(newValue);
}

void TimeInputWidget::mouseDoubleClickEvent(QMouseEvent* event) {
  if(m_isReadOnly) {
    return;
  }
  TimeInputDialog dialog{m_time, Ref(*m_userProfile), this};
  if(dialog.exec() == QDialog::Rejected) {
    return;
  }
  auto newValue = dialog.GetTime();
  SetTime(newValue);
}
