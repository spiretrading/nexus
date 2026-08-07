#ifndef SPIRE_CHECK_BUTTON_MENU_ITEM_HPP
#define SPIRE_CHECK_BUTTON_MENU_ITEM_HPP
#include <QImage>
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/ClickObserver.hpp"

namespace Spire {

  /** Displays a check-button menu item. */
  class CheckButtonMenuItem : public QWidget {
    public:

      /**
       * Constructs a CheckButtonMenuItem with a default current.
       * @param label The displayed text.
       * @param parent The parent widget.
       */
      explicit CheckButtonMenuItem(QString label, QWidget* parent = nullptr);

      /**
       * Constructs a CheckButtonMenuItem.
       * @param label The displayed text.
       * @param current Whether the item is checked.
       * @param parent The parent widget.
       */
      CheckButtonMenuItem(QString label, std::shared_ptr<BooleanModel> current,
        QWidget* parent = nullptr);

      /**
       * Constructs a CheckButtonMenuItem with a leading icon.
       * @param icon The displayed icon.
       * @param label The displayed text.
       * @param current Whether the item is checked.
       * @param parent The parent widget.
       */
      CheckButtonMenuItem(QImage icon, QString label,
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
