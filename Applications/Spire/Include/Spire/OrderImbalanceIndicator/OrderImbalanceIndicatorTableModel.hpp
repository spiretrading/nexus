#ifndef SPIRE_ORDER_IMBALANCE_INDICATOR_TABLE_MODEL_HPP
#define SPIRE_ORDER_IMBALANCE_INDICATOR_TABLE_MODEL_HPP
#include "Beam/TimeService/TimeClient.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/Ui/ArrayTableModel.hpp"

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
        std::shared_ptr<OrderImbalanceIndicatorModel> source,
        std::shared_ptr<Beam::TimeService::TimeClient> time_client);
  
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
      struct Imbalance {
        int m_row_index;
        Nexus::OrderImbalance m_imbalance;
      };
      std::shared_ptr<OrderImbalanceIndicatorModel> m_source;
      ArrayTableModel m_table_model;
      std::shared_ptr<Beam::TimeService::TimeClient> m_time_client;
      TimeInterval m_interval;
      boost::signals2::scoped_connection m_subscription_connection;
      QtPromise<std::vector<Nexus::OrderImbalance>> m_load;
      std::unordered_map<Nexus::Security, Imbalance> m_imbalances;

      std::vector<std::any> make_row(const Nexus::OrderImbalance& imbalance);
      void set_row(int index, const Nexus::OrderImbalance& imbalance);
      void on_imbalance(const Nexus::OrderImbalance& imbalance);
      void on_load(const std::vector<Nexus::OrderImbalance>& imbalances);
  };
}

#endif