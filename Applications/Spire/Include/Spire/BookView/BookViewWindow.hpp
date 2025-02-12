#ifndef SPIRE_BOOK_VIEW_WINDOW_HPP
#define SPIRE_BOOK_VIEW_WINDOW_HPP
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/BookView/BookViewPropertiesWindowFactory.hpp"
#include "Spire/BookView/MarketDepth.hpp"
#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Ui/Window.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Display the book view window for a security. */
  class BookViewWindow : public Window {
    public:

      /**
       * Signals that a cancellation operation is emitted.
       * @param operation The cancellation operation.
       * @param security The security for which orders will be canceled.
       * @param order_key The orders matching order_key will be canceled.
       */
      using CancelOrderSignal = Signal<void (
        CancelKeyBindingsModel::Operation operation,
        const Nexus::Security& security,
        const boost::optional<std::tuple<Nexus::Destination, Nexus::Money>>&
          order_key)>;

      /**
       * The type of function used to build a BookViewModel based on
       * the security.
       * @param security The security that the window is representing.
       * @return the TimeAndSalesModel.
       */
      using ModelBuilder = std::function<
        std::shared_ptr<BookViewModel> (const Nexus::Security& security)>;

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

      /** Connects a slot to the CancelOrderSignal. */
      boost::signals2::connection connect_cancel_order_signal(
        const CancelOrderSignal::slot_type& slot) const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;

    private:
      mutable CancelOrderSignal m_cancel_order_signal;
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
      std::shared_ptr<BookViewPropertiesWindowFactory> m_factory;
      ModelBuilder m_model_builder;
      Nexus::MarketDatabase m_markets;
      std::shared_ptr<QuantityModel> m_default_bid_quantity;
      std::shared_ptr<QuantityModel> m_default_ask_quantity;
      std::shared_ptr<BookViewModel> m_model;
      std::shared_ptr<BooleanModel> m_is_cancel_on_fill;
      std::shared_ptr<BookQuoteModel> m_selected_quote;
      TransitionView* m_transition_view;
      SecurityView* m_security_view;
      boost::signals2::scoped_connection m_bid_order_connection;
      boost::signals2::scoped_connection m_ask_order_connection;

      void on_context_menu(MarketDepth* market_depth, const QPoint& pos);
      void on_cancel_most_recent(const Nexus::BookQuote& book_quote);
      void on_cancel_all(const Nexus::BookQuote& book_quote);
      void on_properties_menu();
      void on_current(const Nexus::Security& security);
      void on_order_operation(Nexus::Side side,
        const ListModel<BookViewModel::UserOrder>::Operation& operation);
  };
}

#endif
