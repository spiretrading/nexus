#ifndef SPIRE_BLOTTER_ORDER_LOG_VIEW_HPP
#define SPIRE_BLOTTER_ORDER_LOG_VIEW_HPP
#include <vector>
#include <QKeySequence>
#include <QWidget>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays the blotter's order log. */
  class BlotterOrderLogView : public QWidget {
    public:

      /** The key sequence to cancel selected orders. */
      const static QKeySequence CANCEL_KEY_SEQUENCE;

      /**
       * Signals to cancel a list of orders.
       * @param orders The list of orders to cancel.
       */
      using CancelSignal = Signal<void (
        const std::vector<Nexus::OrderExecutionService::Order*>& orders)>;

      /**
       * Constructs a BlotterOrderLogView.
       * @param orders The list of orders to display.
       * @param parent The parent widget.
       */
      explicit BlotterOrderLogView(
        std::shared_ptr<OrderListModel> orders, QWidget* parent = nullptr);

      /** Returns the list of orders displayed. */
      const std::shared_ptr<OrderListModel>& get_orders() const;

      /** Connects a slot to the CancelSignal. */
      boost::signals2::connection connect_cancel_signal(
        const CancelSignal::slot_type& slot) const;

    private:
      mutable CancelSignal m_cancel_signal;
      std::shared_ptr<OrderListModel> m_orders;
  };
}

#endif