#ifndef SPIRE_HOTKEY_OVERRIDE_HPP
#define SPIRE_HOTKEY_OVERRIDE_HPP
#include <QAbstractNativeEventFilter>

namespace Spire {

  /**
   * Overrides the native hotkeys used by the operating system. Only one
   * instance of this class needs to be instantiated while the application
   * starts up.
   */
  class HotkeyOverride : public QAbstractNativeEventFilter {
    public:

      /** Constructs a HotkeyOverride. */
      HotkeyOverride() = default;

      bool nativeEventFilter(
        const QByteArray& event_type, void* message, long* result) override;
  };
}

#endif
