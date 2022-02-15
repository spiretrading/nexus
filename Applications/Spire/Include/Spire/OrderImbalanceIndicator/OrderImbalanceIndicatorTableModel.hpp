#ifndef SPIRE_ORDER_IMBALANCE_INDICATOR_TABLE_MODEL_HPP
#define SPIRE_ORDER_IMBALANCE_INDICATOR_TABLE_MODEL_HPP
#include <QTimer>
#include <Beam/TimeService/TimeClientBox.hpp>
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
       * Constructs an empty OrderImbalanceIndicatorTableModel with a given
       * source OrderImbalanceIndicatorModel.
       * @param source The source OrderImbalanceIndicatorModel.
       * @param clock Clock used to determine the current time for dynamic
       *              intervals.
       */
      OrderImbalanceIndicatorTableModel(
        std::shared_ptr<OrderImbalanceIndicatorModel> source,
        Beam::TimeService::TimeClientBox clock);
  
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
      Beam::TimeService::TimeClientBox m_clock;
      TimeInterval m_interval;
      boost::posix_time::time_duration m_offset;
      boost::signals2::scoped_connection m_subscription_connection;
      QtPromise<std::vector<Nexus::OrderImbalance>> m_load;
      std::unordered_map<Nexus::Security, Imbalance> m_imbalances;
      QTimer m_expiration_timer;

      void insert_imbalance(
        std::unordered_map<Nexus::Security, Imbalance>& imbalances,
        const Nexus::OrderImbalance& imbalance);
      std::vector<std::any> make_row(
        const Nexus::OrderImbalance& imbalance) const;
      std::vector<boost::optional<std::any>> make_row_update(
        const Nexus::OrderImbalance& current,
        const Nexus::OrderImbalance& previous) const;
      void set_row(
        const Nexus::OrderImbalance& current, Imbalance& previous);
      void on_expiration_timeout();
      void on_imbalance(const Nexus::OrderImbalance& imbalance);
      void on_load(const std::vector<Nexus::OrderImbalance>& imbalances);
  };
}

#endif
