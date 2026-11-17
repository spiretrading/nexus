#ifndef SPIRE_ORDER_LOG_VIEW_HPP
#define SPIRE_ORDER_LOG_VIEW_HPP
#include <QWidget>
#include "Spire/Blotter/OrdersModel.hpp"

namespace Spire {

  /** Displays an order log. */
  class OrderLogView : public QWidget {
    public:

      /**
       * Constructs a OrderLogView.
       * @param orders The orders to display.
       * @param parent The parent widget.
       */
      explicit OrderLogView(
        std::shared_ptr<OrdersModel> orders, QWidget* parent = nullptr);

      /** Returns the orders displayed in the order log. */
      const std::shared_ptr<OrdersModel>& get_orders() const;

    private:
      std::shared_ptr<OrdersModel> m_orders;
  };
}

#endif
