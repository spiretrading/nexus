#include "Spire/KeyBindings/HotkeyOverride.hpp"
#include <QApplication>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QWidget>
#include <QWindow>
#ifdef Q_OS_WIN
  #include <windows.h>
#endif

using namespace Spire;

namespace {

#ifdef Q_OS_WIN
  Qt::KeyboardModifiers get_qt_modifiers() {
    auto modifiers = Qt::KeyboardModifiers(Qt::NoModifier);
    if((GetKeyState(VK_MENU) & 0x8000) != 0) {
      modifiers |= Qt::AltModifier;
    }
    if((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
      modifiers |= Qt::ControlModifier;
    }
    if((GetKeyState(VK_SHIFT) & 0x8000) != 0) {
      modifiers |= Qt::ShiftModifier;
    }
    return modifiers;
  }

  bool is_down(int vk) {
    return (GetKeyState(vk) & 0x8000) != 0;
  }

  QObject* pick_key_target() {
    if(auto target = QApplication::focusWidget()) {
      return target;
    }
    if(auto target = QApplication::activeWindow()) {
      return target;
    }
    if(auto target = QGuiApplication::focusWindow()) {
      return target;
    }
    return qApp;
  }

  void send_synthetic_key(int qt_key, Qt::KeyboardModifiers modifiers) {
    auto target = pick_key_target();
    auto press = QKeyEvent(QEvent::KeyPress, qt_key, modifiers);
    QCoreApplication::sendEvent(target, &press);
    auto release = QKeyEvent(QEvent::KeyRelease, qt_key, modifiers);
    QCoreApplication::sendEvent(target, &release);
  }
#endif
}

bool HotkeyOverride::nativeEventFilter(
  const QByteArray&, void* message, long* result) {
#ifndef Q_OS_WIN
  Q_UNUSED(message);
  Q_UNUSED(result);
  return false;
#else
  if(!message) {
    return false;
  }
  auto win_message = static_cast<MSG*>(message);
  auto code = win_message->message;
  if(code != WM_KEYDOWN && code != WM_SYSKEYDOWN) {
    return false;
  }
  auto vk = static_cast<UINT>(win_message->wParam);
  auto is_alt = is_down(VK_MENU);
  auto is_ctrl = is_down(VK_CONTROL);
  auto is_shift = is_down(VK_SHIFT);
  auto qt_key = 0;
  if(is_alt && !is_ctrl && !is_shift && vk == VK_F4) {
    qt_key = Qt::Key_F4;
  } else if(is_shift && !is_ctrl && !is_alt && vk == VK_F10) {
    qt_key = Qt::Key_F10;
  } else if(is_shift && !is_ctrl && !is_alt && vk == VK_F11) {
    qt_key = Qt::Key_F11;
  } else if(is_ctrl && !is_shift && !is_alt && vk == VK_F11) {
    qt_key = Qt::Key_F11;
  } else if(is_shift && !is_ctrl && !is_alt && vk == VK_F12) {
    qt_key = Qt::Key_F12;
  } else if(is_ctrl && !is_shift && !is_alt && vk == VK_F12) {
    qt_key = Qt::Key_F12;
  } else if(is_alt && !is_ctrl && !is_shift && vk == VK_ESCAPE) {
    qt_key = Qt::Key_Escape;
  } else if(is_ctrl && !is_alt && !is_shift && vk == VK_ESCAPE) {
    qt_key = Qt::Key_Escape;
  } else {
    return false;
  }
  send_synthetic_key(qt_key, get_qt_modifiers());
  if(result) {
    *result = 0;
  }
  return true;
#endif
}
