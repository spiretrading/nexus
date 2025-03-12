#ifndef SPIRE_BOOK_VIEW_WINDOW_HPP
#define SPIRE_BOOK_VIEW_WINDOW_HPP
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/BookView/BookViewPropertiesWindowFactory.hpp"
#include "Spire/BookView/MarketDepth.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasView.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/Window.hpp"

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
       * @return the BookViewModel.
       */
      using ModelBuilder = std::function<
        std::shared_ptr<BookViewModel> (const Nexus::Security& security)>;

      /**
       * Constructs a BookViewWindow.
       * @param user_profile The user's profile.
       * @param securities The set of securities to use.
       * @param key_bindings The user's key bindings.
       * @param markets The database of markets.
       * @param factory The factory used to create a BookViewPropertiesWindow.
       * @param model_builder The ModelBuilder to use.
       * @param parent The parent widget.
       */
      BookViewWindow(Beam::Ref<UserProfile> user_profile,
        std::shared_ptr<SecurityInfoQueryModel> securities,
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
      UserProfile* m_user_profile;
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
      std::shared_ptr<BookViewPropertiesWindowFactory> m_factory;
      ModelBuilder m_model_builder;
      Nexus::MarketDatabase m_markets;
      std::shared_ptr<InteractionsKeyBindingsModel> m_interactions;
      std::shared_ptr<BookViewModel> m_model;
      std::shared_ptr<BookQuoteModel> m_selected_quote;
      TransitionView* m_transition_view;
      SecurityView* m_security_view;
      CondensedCanvasWidget* m_task_entry_panel;
      boost::signals2::scoped_connection m_bid_order_connection;
      boost::signals2::scoped_connection m_ask_order_connection;

      std::unique_ptr<CanvasNode> make_task_node(const CanvasNode& node);
      void display_task_entry_panel(const OrderTaskArguments& arguments);
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
