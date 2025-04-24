#ifndef SPIRE_BOOK_VIEW_PROPERTIES_WINDOW_FACTORY_HPP
#define SPIRE_BOOK_VIEW_PROPERTIES_WINDOW_FACTORY_HPP
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewPropertiesWindow.hpp"

namespace Spire {

  /** A factory used to create the BookViewPropertiesWindow. */
  class BookViewPropertiesWindowFactory {
    public:

      /**
       * Constructs a BookViewPropertiesWindowFactory using the default
       * properties.
       */
      BookViewPropertiesWindowFactory();

      /**
       * Constructs a BookViewPropertiesWindowFactory.
       * @param properties The properties model of the book view.
       */
      explicit BookViewPropertiesWindowFactory(
        std::shared_ptr<BookViewPropertiesModel> properties);

      /** Returns the properties displayed. */
      const std::shared_ptr<BookViewPropertiesModel>& get_properties() const;

      /*
       * Returns the singleton instance of the BookViewPropertiesWindow.
       * @param key_bindings The KeyBindingsModel storing all of the user's
       *        interactions.
       * @param security The security whose interactions are to be displayed.
       * @param markets The market database to use.
       */
      BookViewPropertiesWindow* make(
        std::shared_ptr<KeyBindingsModel> key_bindings,
        const Nexus::Security& security, const Nexus::MarketDatabase& markets);

    private:
      std::shared_ptr<BookViewPropertiesModel> m_properties;
      std::shared_ptr<SecurityModel> m_security;
      std::unique_ptr<BookViewPropertiesWindow> m_properties_window;
  };
}

#endif
