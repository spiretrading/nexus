#include "Spire/UiTests/UiTests.hpp"
#include <vector>
#include <QCursor>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QTimerEvent>
#include <QWidget>
#include "Spire/Ui/InfoTip.hpp"

using namespace Spire;
using namespace Spire::Tests;

InfoTipTestDriver::InfoTipTestDriver()
    : m_is_advancing(false) {
  qApp->installEventFilter(this);
}

InfoTipTestDriver::~InfoTipTestDriver() {
  qApp->removeEventFilter(this);
}

void InfoTipTestDriver::advance() {
  for(auto widget : QApplication::topLevelWidgets()) {
    auto tip = dynamic_cast<InfoTip*>(widget);
    if(!tip) {
      continue;
    }
    auto timers = std::vector<std::pair<QTimer*, int>>();
    for(auto timer : tip->findChildren<QTimer*>(
        QString(), Qt::FindDirectChildrenOnly)) {
      if(timer->isActive()) {
        timers.emplace_back(timer, timer->timerId());
      }
    }
    for(auto& [timer, id] : timers) {
      if(timer->isActive() && timer->timerId() == id) {
        auto event = QTimerEvent(id);
        m_is_advancing = true;
        QApplication::sendEvent(timer, &event);
        m_is_advancing = false;
      }
    }
    for(auto animation : tip->findChildren<QPropertyAnimation*>(
        QString(), Qt::FindDirectChildrenOnly)) {
      if(animation->state() != QAbstractAnimation::Stopped) {
        animation->setCurrentTime(animation->duration());
      }
    }
  }
}

bool InfoTipTestDriver::eventFilter(QObject* watched, QEvent* event) {
  return event->type() == QEvent::Timer && !m_is_advancing;
}

void Spire::Tests::move_cursor(const QWidget& widget, QPoint position) {
  QCursor::setPos(widget.mapToGlobal(position));
  QApplication::processEvents();
}

void Spire::Tests::press_mouse(QWidget& widget, Qt::MouseButton button) {
  auto event = QMouseEvent(QEvent::MouseButtonPress,
    widget.mapFromGlobal(QCursor::pos()), QCursor::pos(), button,
    QApplication::mouseButtons() | button, QApplication::keyboardModifiers());
  QApplication::sendEvent(&widget, &event);
}

void Spire::Tests::release_mouse(QWidget& widget, Qt::MouseButton button) {
  auto event = QMouseEvent(QEvent::MouseButtonRelease,
    widget.mapFromGlobal(QCursor::pos()), QCursor::pos(), button,
    QApplication::mouseButtons() & ~button,
    QApplication::keyboardModifiers());
  QApplication::sendEvent(&widget, &event);
}

void Spire::Tests::press_key(QWidget& widget, Qt::Key key,
    Qt::KeyboardModifiers modifiers, bool is_auto_repeat) {
  auto event =
    QKeyEvent(QEvent::KeyPress, key, modifiers, QString(), is_auto_repeat);
  QApplication::sendEvent(&widget, &event);
}

void Spire::Tests::release_key(QWidget& widget, Qt::Key key,
    Qt::KeyboardModifiers modifiers, bool is_auto_repeat) {
  auto event =
    QKeyEvent(QEvent::KeyRelease, key, modifiers, QString(), is_auto_repeat);
  QApplication::sendEvent(&widget, &event);
}

void Spire::Tests::type_text(QWidget& widget, const QString& text) {
  for(auto character : text) {
    auto key = static_cast<Qt::Key>(character.toUpper().unicode());
    auto press =
      QKeyEvent(QEvent::KeyPress, key, Qt::NoModifier, QString(character));
    QApplication::sendEvent(&widget, &press);
    auto release =
      QKeyEvent(QEvent::KeyRelease, key, Qt::NoModifier, QString(character));
    QApplication::sendEvent(&widget, &release);
  }
}
