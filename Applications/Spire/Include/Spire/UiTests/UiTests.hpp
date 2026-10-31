#ifndef SPIRE_UI_TESTS_HPP
#define SPIRE_UI_TESTS_HPP
#include <utility>
#include <QApplication>
#include <QPoint>
#include <QString>

class QWidget;

namespace Spire::Tests {

  /**
   * Runs a unit test within a QApplication that uses the offscreen platform.
   * @param test The unit test to run.
   */
  template<typename T>
  void run_ui_test(T&& test) {
    char name[] = "UiTests";
    char platform_option[] = "-platform";
    char platform[] = "offscreen";
    char* arguments[] = {name, platform_option, platform, nullptr};
    auto count = 3;
    auto application = QApplication(count, arguments);
    std::forward<T>(test)();
  }

  /**
   * Moves the mouse cursor to a position within a widget.
   * @param widget The widget containing the position.
   * @param position The position in the widget's coordinates.
   */
  void move_cursor(const QWidget& widget, QPoint position);

  /**
   * Sends a mouse button press to a widget at the cursor's position.
   * @param widget The widget to send the press to.
   * @param button The mouse button pressed.
   */
  void press_mouse(QWidget& widget, Qt::MouseButton button);

  /**
   * Sends a mouse button release to a widget at the cursor's position.
   * @param widget The widget to send the release to.
   * @param button The mouse button released.
   */
  void release_mouse(QWidget& widget, Qt::MouseButton button);

  /**
   * Sends a key press to a widget.
   * @param widget The widget to send the key press to.
   * @param key The key pressed.
   * @param modifiers The keyboard modifiers held.
   * @param is_auto_repeat Whether the key press is an auto-repeat.
   */
  void press_key(QWidget& widget, Qt::Key key,
    Qt::KeyboardModifiers modifiers = Qt::NoModifier,
    bool is_auto_repeat = false);

  /**
   * Sends a key release to a widget.
   * @param widget The widget to send the key release to.
   * @param key The key released.
   * @param modifiers The keyboard modifiers held.
   * @param is_auto_repeat Whether the key release is an auto-repeat.
   */
  void release_key(QWidget& widget, Qt::Key key,
    Qt::KeyboardModifiers modifiers = Qt::NoModifier,
    bool is_auto_repeat = false);

  /**
   * Sends the key presses and releases that type text into a widget.
   * @param widget The widget to type into.
   * @param text The text to type.
   */
  void type_text(QWidget& widget, const QString& text);
}

#endif
