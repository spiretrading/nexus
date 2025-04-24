#ifndef SPIRE_BOOK_VIEW_INTERACTION_PROPERTIES_PAGE_HPP
#define SPIRE_BOOK_VIEW_INTERACTION_PROPERTIES_PAGE_HPP
#include <QWidget>
#include "Spire/BookView/BookView.hpp"
#include "Spire/KeyBindings/KeyBindingsModel.hpp"
#include "Spire/Ui/RegionDropDownBox.hpp"
#include "Spire/Ui/SecurityBox.hpp"

namespace Spire {

  /** Implements a widget for the interaction settings. */
  class BookViewInteractionPropertiesPage : public QWidget {
    public:

      /**
       * Constructs a BookViewInteractionPropertiesPage.
       * @param key_bindings The KeyBindingsModel storing all of the user's
       *        interactions.
       * @param security The security whose interactions are to be displayed.
       * @param parent The parent widget.
       */
      BookViewInteractionPropertiesPage(
        std::shared_ptr<KeyBindingsModel> key_bindings,
        std::shared_ptr<SecurityModel> security, QWidget* parent = nullptr);

    private:
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
      std::shared_ptr<RegionModel> m_current_region;

      void on_reset_click();
  };
}

#endif
