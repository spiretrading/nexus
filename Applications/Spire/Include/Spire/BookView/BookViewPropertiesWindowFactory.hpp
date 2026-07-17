#ifndef SPIRE_BOOK_VIEW_PROPERTIES_WINDOW_FACTORY_HPP
#define SPIRE_BOOK_VIEW_PROPERTIES_WINDOW_FACTORY_HPP
#include "Spire/BookView/BookViewPropertiesWindow.hpp"
#include "Spire/Spire/ProxyValueModel.hpp"

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

      /**
       * Pre-constructs the singleton BookViewPropertiesWindow without wiring
       * a preview. Used at login to eliminate first-use lag.
       * @param key_bindings The KeyBindingsModel storing all of the user's
       *        interactions.
       */
      BookViewPropertiesWindow* make(
        std::shared_ptr<KeyBindingsModel> key_bindings);

      /**
       * Returns the singleton BookViewPropertiesWindow, previewing edits live
       * through the given proxy.
       * @param key_bindings The KeyBindingsModel storing all of the user's
       *        interactions.
       * @param ticker The ticker whose interactions are to be displayed.
       * @param live_preview The proxy that receives live edits.
       */
      BookViewPropertiesWindow* make(
        std::shared_ptr<KeyBindingsModel> key_bindings,
        const Nexus::Ticker& ticker,
        std::shared_ptr<ProxyValueModel<BookViewProperties>> live_preview);

    private:
      std::shared_ptr<BookViewPropertiesModel> m_properties;
      std::shared_ptr<TickerModel> m_ticker;
      std::unique_ptr<BookViewPropertiesWindow> m_properties_window;
      std::shared_ptr<ProxyValueModel<BookViewProperties>> m_window_proxy;
      std::shared_ptr<LocalBookViewPropertiesModel> m_preview;
      std::shared_ptr<ProxyValueModel<BookViewProperties>> m_live_preview;
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
      InteractionsKeyBindingsModel m_initial_interactions;
      bool m_are_interactions_detached;
      bool m_has_interactions_snapshot;
      boost::signals2::scoped_connection m_submit_connection;
      boost::signals2::scoped_connection m_cancel_connection;

      void snapshot_interactions();
      void revert_interactions();
      void on_submit();
      void on_cancel();
  };
}

#endif
