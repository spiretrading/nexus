#ifndef SPIRE_ORDER_LOG_VIEW_HPP
#define SPIRE_ORDER_LOG_VIEW_HPP
#include <QWidget>
#include "Spire/Blotter/OrdersModel.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsModel.hpp"

namespace Spire {
  class TableSelectionModel;
  class TableView;

  /** Displays an order log. */
  class OrderLogView : public QWidget {
    public:

      /**
       * Signals a request to cancel the selected orders.
       * @param orders The orders selected when cancellation was requested.
       */
      using CancelSignal = Signal<
        void (const std::vector<std::shared_ptr<Nexus::Order>>& orders)>;

      /**
       * Signals a key-bound request to cancel orders.
       * @param operation The requested cancel operation.
       */
      using CancelOperationSignal =
        Signal<void (CancelKeyBindingsModel::Operation operation)>;

      /**
       * Constructs a OrderLogView.
       * @param orders The orders to display.
       * @param bindings The user's cancel key bindings.
       * @param parent The parent widget.
       */
      OrderLogView(std::shared_ptr<OrdersModel> orders,
        std::shared_ptr<CancelKeyBindingsModel> bindings,
        QWidget* parent = nullptr);

      /** Returns the orders displayed in the order log. */
      const std::shared_ptr<OrdersModel>& get_orders() const;

      /** Connects a slot to the CancelSignal. */
      boost::signals2::connection connect_cancel_signal(
        const CancelSignal::slot_type& slot) const;

      /** Connects a slot to the CancelOperationSignal. */
      boost::signals2::connection connect_cancel_operation_signal(
        const CancelOperationSignal::slot_type& slot) const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;

    private:
      friend class BlotterWindowSettings;
      mutable CancelSignal m_cancel_signal;
      mutable CancelOperationSignal m_cancel_operation_signal;
      std::shared_ptr<CancelKeyBindingsModel> m_bindings;
      std::shared_ptr<OrdersModel> m_orders;
      std::shared_ptr<TableSelectionModel> m_selection;
      TableView* m_table;
  };
}

#endif
