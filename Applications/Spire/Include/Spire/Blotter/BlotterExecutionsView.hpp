#ifndef SPIRE_BLOTTER_EXECUTIONS_VIEW_HPP
#define SPIRE_BLOTTER_EXECUTIONS_VIEW_HPP
#include <QWidget>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays the blotter's execution reports. */
  class BlotterExecutionsView : public QWidget {
    public:

      /**
       * Constructs a BlotterExecutionsView.
       * @param orders The list of orders whose execution reports are displayed.
       * @param parent The parent widget.
       */
      explicit BlotterExecutionsView(
        std::shared_ptr<OrderListModel> orders, QWidget* parent = nullptr);

      /** Returns the list of orders whose execution reports are displayed. */
      const std::shared_ptr<OrderListModel>& get_orders() const;

    private:
      std::shared_ptr<OrderListModel> m_orders;
  };
}

#endif
