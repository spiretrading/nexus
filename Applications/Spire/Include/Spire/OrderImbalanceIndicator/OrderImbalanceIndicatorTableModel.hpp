#ifndef SPIRE_ORDER_IMBALANCE_INDICATOR_TABLE_MODEL_HPP
#define SPIRE_ORDER_IMBALANCE_INDICATOR_TABLE_MODEL_HPP
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorTableModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModelSubscriber.hpp"
#include "Spire/Ui/ArrayTableModel.hpp"

namespace Spire {

  /**
   * Implements a TableModel for OrderImbalances published within a fixed time
   * interval.
   */
  class OrderImbalanceIndicatorTableModel : public TableModel {
    public:

      /**
       * Constructs an OrderImbalanceIndicatorTableModel with a subscription
       * for the given interval.
       * @param interval The time interval to subscribe to.
       * @param source The source OrderImbalance model.
       */
      OrderImbalanceIndicatorTableModel(TimeInterval interval,
        std::shared_ptr<OrderImbalanceIndicatorModel> source);

      int get_row_size() const override;

      int get_column_size() const override;

      const std::any& at(int row, int column) const override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

    private:
      LocalOrderImbalanceIndicatorTableModel m_table;
      std::shared_ptr<OrderImbalanceIndicatorModel> m_source;
      boost::signals2::scoped_connection m_subscription_connection;
      QtPromise<void> m_load;

      void on_imbalance(const Nexus::OrderImbalance& imbalance);
      void on_load(const std::vector<Nexus::OrderImbalance>& imbalances);
  };
}

#endif
