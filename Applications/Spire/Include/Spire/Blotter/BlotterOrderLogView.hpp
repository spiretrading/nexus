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

      /**
       * Constructs a BlotterOrderLogView.
       * @param orders The list of orders to display.
       * @param parent The parent widget.
       */
      explicit BlotterOrderLogView(
        std::shared_ptr<OrderListModel> orders, QWidget* parent = nullptr);

      /** Returns the list of orders displayed. */
      const std::shared_ptr<OrderListModel>& get_orders() const;

    private:
      std::shared_ptr<OrderListModel> m_orders;
  };
}

#endif
