#ifndef SPIRE_CURRENT_ORDER_IMBALANCE_INDICATOR_TABLE_MODEL_HPP
#define SPIRE_CURRENT_ORDER_IMBALANCE_INDICATOR_TABLE_MODEL_HPP
#include <Beam/Queues/CallbackQueue.hpp>
#include <Beam/Threading/TimerBox.hpp>
#include <Beam/TimeService/TimeClientBox.hpp>
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorTableModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"

namespace Spire {

  /**
   * Implements a TableModel that contains OrderImbalances published within
   * a dynamic interval between a duration from the present and the present
   * time.
   */
  class CurrentOrderImbalanceIndicatorTableModel : public TableModel {
    public:

      using TimerFactory = std::function<
        Beam::Threading::TimerBox (boost::posix_time::time_duration duration)>;

      /**
       * Constructs a CurrentOrderImbalanceIndicatorTableModel.
       * @param offset The offset from the present time.
       * @param clock The clock used to determine the present time.
       * @param timer_factory Function used to create the timers used
       *                      to remove expired OrderImbalances.
       * @param source The source OrderImbalance model.
       */
      CurrentOrderImbalanceIndicatorTableModel(
        boost::posix_time::time_duration offset,
        Beam::TimeService::TimeClientBox clock, TimerFactory timer_factory,
        std::shared_ptr<OrderImbalanceIndicatorModel> source);

      int get_row_size() const override;

      int get_column_size() const override;

      const std::any& at(int row, int column) const override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<OrderImbalanceIndicatorModel> m_source;
      LocalOrderImbalanceIndicatorTableModel m_table;
      boost::signals2::scoped_connection m_subscription_connection;
      QtPromise<void> m_load;
      boost::posix_time::time_duration m_offset;
      Beam::TimeService::TimeClientBox m_clock;
      TimerFactory m_timer_factory;
      Beam::CallbackQueue m_timer_queue;
      std::unique_ptr<Beam::Threading::TimerBox> m_timer;
      Nexus::Security m_next_expiring;

      void update_next_expiring();
      void on_expiration_timeout(Beam::Threading::Timer::Result result);
      void on_imbalance(const Nexus::OrderImbalance& imbalance);
      void on_load(const std::vector<Nexus::OrderImbalance>& imbalances);
  };
}

#endif
