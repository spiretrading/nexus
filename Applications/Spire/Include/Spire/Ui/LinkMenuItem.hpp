#ifndef SPIRE_LINK_MENU_ITEM_HPP
#define SPIRE_LINK_MENU_ITEM_HPP
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/ClickObserver.hpp"

namespace Spire {

  /** Displays a check-button menu item identifying a linkable window. */
  class LinkMenuItem : public QWidget {
    public:

      /** Enumerates the types of linkable windows. */
      enum class LinkableWindowType {

        /** A book view window. */
        BOOK_VIEW,

        /** A chart window. */
        CHART,

        /** A time and sales window. */
        TIME_AND_SALES
      };

      /**
       * Constructs a LinkMenuItem with a default current.
       * @param type The type of window.
       * @param symbol The ticker symbol displayed by the window.
       * @param parent The parent widget.
       */
      LinkMenuItem(LinkableWindowType type, QString symbol,
        QWidget* parent = nullptr);

      /**
       * Constructs a LinkMenuItem.
       * @param type The type of window.
       * @param symbol The ticker symbol displayed by the window.
       * @param current Whether the item is checked.
       * @param parent The parent widget.
       */
      LinkMenuItem(LinkableWindowType type, QString symbol,
        std::shared_ptr<BooleanModel> current, QWidget* parent = nullptr);

      /** Returns the current model. */
      const std::shared_ptr<BooleanModel>& get_current() const;

    private:
      std::shared_ptr<BooleanModel> m_current;
      ClickObserver m_click_observer;

      void on_click();
  };
}

#endif
