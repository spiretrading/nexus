#ifndef SPIRE_BOOK_VIEW_WINDOW_HPP
#define SPIRE_BOOK_VIEW_WINDOW_HPP
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/BookView/BookViewPropertiesWindowFactory.hpp"
#include "Spire/BookView/MarketDepth.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasView.hpp"
#include "Spire/LegacyUI/PersistentWindow.hpp"
#include "Spire/LegacyUI/SecurityContext.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Ui/SecurityBox.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Display the book view window for a security. */
  class BookViewWindow : public Window, public LegacyUI::PersistentWindow,
      public LegacyUI::SecurityContext {
    public:

      /**
       * Signals that a task has been submitted to be run.
       * @param task The CanvasNode representing the task to run.
       */
      using SubmitTaskSignal =
        Signal<void (const std::shared_ptr<CanvasNode>& task)>;

      /** Specifies the criteria to match when canceling tasks. */
      struct CancelCriteria {

        /** The destination to match. */
        Nexus::Destination m_destination;

        /** The price to match. */
        Nexus::Money m_price;
      };

      /**
       * Signals that a cancellation operation is emitted.
       * @param operation The cancellation operation.
       * @param security The security for which orders will be canceled.
       * @param criteria The criteria of the tasks to cancel.
       */
      using CancelOperationSignal = Signal<void (
        CancelKeyBindingsModel::Operation operation,
        const Nexus::Security& security,
        const boost::optional<CancelCriteria>& criteria)>;

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

      /**
       * Constructs a BookViewWindow.
       * @param user_profile The user's profile.
       * @param securities The set of securities to use.
       * @param key_bindings The user's key bindings.
       * @param markets The database of markets.
       * @param factory The factory used to create a BookViewPropertiesWindow.
       * @param model_builder The ModelBuilder to use.
       * @param identifier The SecurityContext identifier.
       * @param parent The parent widget.
       */
      BookViewWindow(Beam::Ref<UserProfile> user_profile,
        std::shared_ptr<SecurityInfoQueryModel> securities,
        std::shared_ptr<KeyBindingsModel> key_bindings,
        Nexus::MarketDatabase markets,
        std::shared_ptr<BookViewPropertiesWindowFactory> factory,
        ModelBuilder model_builder, std::string identifier,
        QWidget* parent = nullptr);

      /** Returns the currently displayed security. */
      const std::shared_ptr<SecurityModel>& get_current() const;

      /** Connects a slot to the SubmitTaskSignal. */
      boost::signals2::connection connect_submit_task_signal(
        const SubmitTaskSignal::slot_type& slot) const;

      /** Connects a slot to the CancelOperationSignal. */
      boost::signals2::connection connect_cancel_operation_signal(
        const CancelOperationSignal::slot_type& slot) const;

      std::unique_ptr<LegacyUI::WindowSettings>
        GetWindowSettings() const override;

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void showEvent(QShowEvent* event) override;
      void HandleLink(SecurityContext& context) override;
      void HandleUnlink() override;

    private:
      friend class BookViewWindowSettings;
      friend class LegacyBookViewWindowSettings;
      mutable SubmitTaskSignal m_submit_task_signal;
      mutable CancelOperationSignal m_cancel_operation_signal;
      UserProfile* m_user_profile;
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
      std::shared_ptr<BookViewPropertiesWindowFactory> m_factory;
      ModelBuilder m_model_builder;
      Nexus::MarketDatabase m_markets;
      std::shared_ptr<InteractionsKeyBindingsModel> m_interactions;
      std::shared_ptr<BookViewModel> m_model;
      MarketDepth* m_market_depth;
      TransitionView* m_transition_view;
      std::string m_link_identifier;
      boost::signals2::scoped_connection m_link_connection;
      SecurityView* m_security_view;
      CondensedCanvasWidget* m_task_entry_panel;
      bool m_is_task_entry_panel_for_interactions;
      boost::signals2::scoped_connection m_bid_order_connection;
      boost::signals2::scoped_connection m_ask_order_connection;

      std::unique_ptr<CanvasNode> make_task_node(const CanvasNode& node);
      void display_interactions_panel();
      void display_task_entry_panel(const OrderTaskArguments& arguments);
      void remove_task_entry_panel();
      void on_context_menu(const QPoint& pos);
      void on_task_entry_key_press(const QKeyEvent& event);
      void on_cancel_most_recent(
        const MarketDepth::CurrentUserOrder& user_order);
      void on_cancel_all(const MarketDepth::CurrentUserOrder& user_order);
      void on_properties_menu();
      void on_current(const Nexus::Security& security);
      void on_order_operation(Nexus::Side side,
        const ListModel<BookViewModel::UserOrder>::Operation& operation);
  };
}

#endif
