#ifndef SPIRE_HOTKEYOVERRIDE_HPP
#define SPIRE_HOTKEYOVERRIDE_HPP
#include <QWidget>

namespace Spire {

  /*! \class HotkeyOverride
      \brief Overrides the native hotkeys used by the operating system. Only
             one instance of this class needs to be instantiated while the
             application starts up.
   */
  class HotkeyOverride : public QWidget {
    public:

      //! Constructs a HotkeyOverride.
      HotkeyOverride();

      virtual ~HotkeyOverride();

    protected:
      virtual bool nativeEvent(const QByteArray& eventType, void* message,
        long* result);
  };
}

#endif
