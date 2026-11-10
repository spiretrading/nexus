#include "Spire/UiTests/UiTests.hpp"
#include <QCursor>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWidget>

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
