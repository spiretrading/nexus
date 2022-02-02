#ifndef SPIRE_ORDER_IMBALANCE_INDICATOR_TABLE_MODEL_HPP
#define SPIRE_ORDER_IMBALANCE_INDICATOR_TABLE_MODEL_HPP
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/Ui/TableModel.hpp"

namespace Spire {

  /**
   * Implements a TableModel using an OrderImbalanceIndicatorModel as a source
   * model.
   */
  class OrderImbalanceIndicatorTableModel : public TableModel {
    public:

      /**
       * Constructs an empty OrderImbalanceIndicatorTableModel with
       * a given source OrderImbalanceIndicatorModel.
       * @param source The source OrderImbalanceIndicatorModel.
       */
      OrderImbalanceIndicatorTableModel(
        std::shared_ptr<OrderImbalanceIndicatorModel> source);
  
      /**
       * Replaces the model's current OrderImbalances with the imbalances
       * published within the given interval.
       * @param interval The time inverval.
       */
      void set_interval(const TimeInterval& interval);
  
      /**
       * Replaces the model's current OrderImbalances with the imbalances
       * published within the dynamic interval [present - offset, present]
       * Imbalances older than present - offset are removed as they expire.
       * @param offset The time duration from present.
       */
      void set_offset(const boost::posix_time::time_duration& offset);

      int get_row_size() const override;

      int get_column_size() const override;

      const std::any& at(int row, int column) const override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<OrderImbalanceIndicatorModel> m_source;
      OperationSignal m_operation_signal;
  };
}

#endif
