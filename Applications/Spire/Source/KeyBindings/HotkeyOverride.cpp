#include "Spire/KeyBindings/HotkeyOverride.hpp"
#ifdef _MSC_VER
  #include <windows.h>
#endif
#include <boost/optional/optional.hpp>
#include <QApplication>
#include <QKeyEvent>
#include <QWindow>

using namespace boost;
using namespace Spire;

namespace {
  const auto HOTKEY_ALT_F4_INDEX = 100;
  const auto HOTKEY_SHIFT_F11_INDEX = 101;
  const auto HOTKEY_SHIFT_F12_INDEX = 102;
  const auto HOTKEY_CTRL_F11_INDEX = 103;
  const auto HOTKEY_CTRL_F12_INDEX = 104;
  const auto HOTKEY_ALT_ESC_INDEX = 105;
  const auto HOTKEY_CTRL_ESC_INDEX = 106;
}

HotkeyOverride::HotkeyOverride() {
#ifdef _MSC_VER
  auto handle = reinterpret_cast<HWND>(winId());
  ::RegisterHotKey(handle, HOTKEY_ALT_F4_INDEX, MOD_ALT, VK_F4);
  ::RegisterHotKey(handle, HOTKEY_SHIFT_F11_INDEX, MOD_SHIFT, VK_F11);
  ::RegisterHotKey(handle, HOTKEY_SHIFT_F12_INDEX, MOD_SHIFT, VK_F12);
  ::RegisterHotKey(handle, HOTKEY_CTRL_F11_INDEX, MOD_CONTROL, VK_F11);
  ::RegisterHotKey(handle, HOTKEY_CTRL_F12_INDEX, MOD_CONTROL, VK_F12);
  ::RegisterHotKey(handle, HOTKEY_ALT_ESC_INDEX, MOD_ALT, VK_ESCAPE);
  ::RegisterHotKey(handle, HOTKEY_CTRL_ESC_INDEX, MOD_CONTROL, VK_ESCAPE);
#endif
}

HotkeyOverride::~HotkeyOverride() {
#ifdef _MSC_VER
  auto handle = reinterpret_cast<HWND>(winId());
  ::UnregisterHotKey(handle, 100);
#endif
}

bool HotkeyOverride::nativeEvent(
    const QByteArray& event_type, void* message, long* result) {
#ifdef _MSC_VER
  auto win_message = static_cast<MSG*>(message);
  if(win_message->message == WM_HOTKEY) {
    auto event = [&] () -> optional<QKeyEvent> {
      if(win_message->wParam == HOTKEY_ALT_F4_INDEX) {
        Qt::KeyboardModifiers modifiers = Qt::AltModifier;
        return QKeyEvent(QEvent::KeyPress, Qt::Key_F4, modifiers);
      } else if(win_message->wParam == HOTKEY_SHIFT_F11_INDEX) {
        Qt::KeyboardModifiers modifiers = Qt::ShiftModifier;
        return QKeyEvent(QEvent::KeyPress, Qt::Key_F11, modifiers);
      } else if(win_message->wParam == HOTKEY_SHIFT_F12_INDEX) {
        Qt::KeyboardModifiers modifiers = Qt::ShiftModifier;
        return QKeyEvent(QEvent::KeyPress, Qt::Key_F12, modifiers);
      } else if(win_message->wParam == HOTKEY_CTRL_F11_INDEX) {
        Qt::KeyboardModifiers modifiers = Qt::ControlModifier;
        return QKeyEvent(QEvent::KeyPress, Qt::Key_F11, modifiers);
      } else if(win_message->wParam == HOTKEY_CTRL_F12_INDEX) {
        Qt::KeyboardModifiers modifiers = Qt::ControlModifier;
        return QKeyEvent(QEvent::KeyPress, Qt::Key_F12, modifiers);
      } else if(win_message->wParam == HOTKEY_ALT_ESC_INDEX) {
        Qt::KeyboardModifiers modifiers = Qt::AltModifier;
        return QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, modifiers);
      } else if(win_message->wParam == HOTKEY_CTRL_ESC_INDEX) {
        Qt::KeyboardModifiers modifiers = Qt::ControlModifier;
        return QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, modifiers);
      }
      return none;
    }();
    if(event) {
      auto receiver = QApplication::focusWidget();
      if(receiver == nullptr) {
        receiver = QApplication::activeWindow();
      }
      if(receiver != nullptr) {
        QApplication::sendEvent(receiver, &*event);
      }
    }
    *result = 0;
    return true;
  }
#endif
  return false;
}
