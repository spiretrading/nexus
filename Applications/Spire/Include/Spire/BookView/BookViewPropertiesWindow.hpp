#ifndef BOOK_VIEW_PROPERTIES_WINDOW_HPP
#define BOOK_VIEW_PROPERTIES_WINDOW_HPP
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/KeyBindings/KeyBindingsModel.hpp"
#include "Spire/Ui/NavigationView.hpp"
#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Displays the properties for the book view window. */
  class BookViewPropertiesWindow : public Window {
    public:

      /**
       * Constructs a BookViewPropertiesWindow.
       * @param properties The initial properties.
       * @param key_bindings The user's key bindings.
       * @param security The security whose interactions are to be displayed.
       * @param parent The parent widget.
       */
      BookViewPropertiesWindow(
        std::shared_ptr<BookViewPropertiesModel> properties,
        std::shared_ptr<KeyBindingsModel> key_bindings,
        std::shared_ptr<SecurityModel> security, QWidget* parent = nullptr);

    private:
      std::shared_ptr<BookViewPropertiesModel> m_properties;
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
      std::shared_ptr<SecurityModel> m_security;
      BookViewProperties m_initial_properties;
      InteractionsKeyBindingsModel m_initial_interactions;
      bool m_are_interactions_detached;
      NavigationView* m_navigation_view;
      QWidget* m_highlights_page;
      boost::signals2::scoped_connection m_level_connection;
      boost::signals2::scoped_connection m_security_connection;

      void on_cancel_button_click();
      void on_done_button_click();
      void on_level_update(const BookViewLevelProperties& properties);
      void on_security_update(const Nexus::Security& security);
  };
}

#endif
