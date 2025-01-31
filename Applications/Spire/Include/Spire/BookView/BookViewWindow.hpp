#ifndef SPIRE_BOOK_VIEW_WINDOW_HPP
#define SPIRE_BOOK_VIEW_WINDOW_HPP
#include <boost/signals2/connection.hpp>
#include <QTimer>
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/BookView/BookViewPropertiesWindowFactory.hpp"
#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Ui/Window.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Display the book view window for a security. */
  class BookViewWindow : public Window {
    public:

      /**
       * Signals that orders are being canceled.
       * @param operation The cancel operation.
       * @param orders The orders to be canceled.
       */
      using CancelSignal = Signal<void (
        CancelKeyBindingsModel::Operation operation,
        const std::vector<Nexus::OrderExecutionService::OrderFields>& orders)>;

      /**
       * The type of function used to build a BookViewModel based on
       * the security.
       * @param security The security that the window is representing.
       * @return the TimeAndSalesModel.
       */
      using ModelBuilder = std::function<
        std::shared_ptr<BookViewModel>(const Nexus::Security& security)>;

      /**
       * Constructs a TimeAndSalesWindow.
       * @param securities The set of securities to use.
       * @param key_bindings The KeyBindingsModel storing all of the user's
       *        interactions.
       * @param markets The database of markets.
       * @param factory The factory used to create a BookViewPropertiesWindow.
       * @param model_builder The ModelBuilder to use.
       * @param parent The parent widget.
       */
      BookViewWindow(std::shared_ptr<SecurityInfoQueryModel> securities,
        std::shared_ptr<KeyBindingsModel> key_bindings,
        Nexus::MarketDatabase markets,
        std::shared_ptr<BookViewPropertiesWindowFactory> factory,
        ModelBuilder model_builder, QWidget* parent = nullptr);

      /** Connects a slot to the CancelSignal. */
      boost::signals2::connection connect_cancel_signal(
        const CancelSignal::slot_type& slot) const;

    private:
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
      std::shared_ptr<BookViewPropertiesWindowFactory> m_factory;
      ModelBuilder m_model_builder;
      Nexus::MarketDatabase m_markets;
      std::shared_ptr<QuantityModel> m_default_bid_quantity;
      std::shared_ptr<QuantityModel> m_default_ask_quantity;
      TransitionView* m_transition_view;
      SecurityView* m_security_view;
      boost::signals2::scoped_connection m_properties_connection;

      void on_context_menu(MarketDepth* market_depth, const QPoint& pos);
      void on_cancel_most_recent(const Nexus::BookQuote& book_quote);
      void on_cancel_all(const Nexus::BookQuote& book_quote);
      void on_properties_menu();
      void on_current(const Nexus::Security& security);
      //void on_properties(const TimeAndSalesProperties& properties);
  };
}

#endif
