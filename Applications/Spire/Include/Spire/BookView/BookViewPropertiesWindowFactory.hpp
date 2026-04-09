#ifndef SPIRE_BOOK_VIEW_PROPERTIES_WINDOW_FACTORY_HPP
#define SPIRE_BOOK_VIEW_PROPERTIES_WINDOW_FACTORY_HPP
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
       * @param ticker The ticker whose interactions are to be displayed.
       */
      BookViewPropertiesWindow* make(
        std::shared_ptr<KeyBindingsModel> key_bindings,
        const Nexus::Ticker& ticker);

    private:
      std::shared_ptr<BookViewPropertiesModel> m_properties;
      std::shared_ptr<TickerModel> m_ticker;
      std::unique_ptr<BookViewPropertiesWindow> m_properties_window;
  };
}

#endif
