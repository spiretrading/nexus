#ifndef SPIRE_HOTKEY_OVERRIDE_HPP
#define SPIRE_HOTKEY_OVERRIDE_HPP
#include <QWidget>

namespace Spire {

  /**
   * Overrides the native hotkeys used by the operating system. Only one
   * instance of this class needs to be instantiated while the application
   * starts up.
   */
  class HotkeyOverride : public QWidget {
    public:

      /** Constructs a HotkeyOverride. */
      HotkeyOverride();

      ~HotkeyOverride() override;

    protected:
      bool nativeEvent(
        const QByteArray& event_type, void* message, long* result) override;
  };
}

#endif
