#ifndef SPIRE_BLOTTER_ORDER_LOG_VIEW_HPP
#define SPIRE_BLOTTER_ORDER_LOG_VIEW_HPP
#include <QWidget>
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Spire/Spire/ListModel.hpp"

namespace Spire {

  /** An observable list of orders. */
  using OrdersModel = ListModel<std::shared_ptr<Nexus::Order>>;

  /** Displays a blotter's order log. */
  class BlotterOrderLogView : public QWidget {
    public:

      /**
       * Constructs a BlotterOrderLogView.
       * @param orders The orders to display.
       * @param parent The parent widget.
       */
      explicit BlotterOrderLogView(
        std::shared_ptr<OrdersModel> orders, QWidget* parent = nullptr);

      /** Returns the orders displayed in the order log. */
      const std::shared_ptr<OrdersModel>& get_orders() const;

    private:
      std::shared_ptr<OrdersModel> m_orders;
  };
}

#endif
