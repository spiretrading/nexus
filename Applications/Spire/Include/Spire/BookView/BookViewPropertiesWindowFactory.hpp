#ifndef SPIRE_BOOK_VIEW_PROPERTIES_WINDOW_FACTORY_HPP
#define SPIRE_BOOK_VIEW_PROPERTIES_WINDOW_FACTORY_HPP
#include "Spire/BookView/BookViewPropertiesWindow.hpp"

namespace Spire {

  /** A factory used to create the BookViewPropertiesWindow. */
  class BookViewPropertiesWindowFactory {
    public:

      /**
       * Constructs a BookViewPropertiesWindowFactory using the default
       * model.
       */
      BookViewPropertiesWindowFactory(
        std::shared_ptr<KeyBindingsModel> key_bindings,
        const Nexus::MarketDatabase& markets);

      /**
       * Constructs a BookViewPropertiesWindowFactory.
       * @param properties The properties model of the time and sales.
       */
      explicit BookViewPropertiesWindowFactory(
        std::shared_ptr<BookViewPropertiesModel> properties,
        std::shared_ptr<KeyBindingsModel> key_bindings,
        const Nexus::MarketDatabase& markets);

      /** Returns the properties displayed. */
      const std::shared_ptr<BookViewPropertiesModel>&
        get_properties() const;

      /* Returns the newly created BookViewPropertiesWindow. */
      BookViewPropertiesWindow* make(const Nexus::Security& security);

    private:
      std::shared_ptr<BookViewPropertiesModel> m_properties;
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
      Nexus::MarketDatabase m_markets;
      std::shared_ptr<SecurityModel> m_security;
      std::unique_ptr<BookViewPropertiesWindow> m_properties_window;
  };
}

#endif
