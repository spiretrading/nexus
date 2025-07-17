#ifndef SPIRE_TARGET_MENU_ITEM_HPP
#define SPIRE_TARGET_MENU_ITEM_HPP
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Playback/Playback.hpp"
#include "Spire/Toolbar/ToolbarWindow.hpp"
#include "Spire/Ui/ClickObserver.hpp"

namespace Spire {

  /** Represents a menu item in the replay attach menu. */
  class TargetMenuItem : public QWidget {
    public:

      /** Represents a window group target available for replay attachment. */
      struct Target {

        /** The color associated with the target. */
        boost::optional<QColor> m_color;

        /** The security assigned to the target. */
        boost::optional<Nexus::Security> m_security;

        /** The window types in the group. */
        std::vector<ToolbarWindow::WindowType> m_window_types;
      };

      /**
       * Constructs a TargetMenuItem with the default current.
       * @param target The target of the replay attachment.
       * @param parent The parent widget.
       */
      explicit TargetMenuItem(Target target, QWidget* parent = nullptr);

      /**
       * Constructs a TargetMenuItem.
       * @param target The replay attachment target that the item represents.
       * @param current Whether the item is checked.
       * @param parent The parent widget.
       */
      TargetMenuItem(Target target, std::shared_ptr<BooleanModel> current,
        QWidget* parent = nullptr);

      /** Returns the target. */
      const Target& get_target() const;

      /** Returns the current value, <code>true</code> iff item is checked. */
      const std::shared_ptr<BooleanModel>& get_current() const;

    private:
      std::shared_ptr<BooleanModel> m_current;
      Target m_target;
      ClickObserver m_click_observer;

      void on_click();
  };

  /** Returns the text representation of a Target. */ 
  QString to_text(const TargetMenuItem::Target& target);
}

#endif
