#ifndef SPIRE_ORDER_IMBALANCE_INDICATOR_TABLE_MODEL_HPP
#define SPIRE_ORDER_IMBALANCE_INDICATOR_TABLE_MODEL_HPP
#include <QTimer>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/Ui/ArrayTableModel.hpp"

namespace Spire {

  class microsec_clock {
    public:

      void set_current(boost::posix_time::ptime current) {
        m_current = current;
      }

      //! return a local time object for the given zone, based on computer clock
      template<typename U>
      static boost::posix_time::ptime local_time(boost::shared_ptr<U> time_zone) {
        return universal_time();
      }

      //! Returns the local time based on computer clock settings
      static boost::posix_time::ptime local_time() {
        return universal_time();
      }

      //! Returns the UTC time based on computer settings
      static boost::posix_time::ptime universal_time() {
        return m_current;
      }

    private:
      inline static boost::posix_time::ptime m_current =
        boost::posix_time::from_time_t(0);
  };

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
      struct Imbalance {
        int m_row_index;
        Nexus::OrderImbalance m_imbalance;
      };
      std::shared_ptr<OrderImbalanceIndicatorModel> m_source;
      ArrayTableModel m_table_model;
      TimeInterval m_interval;
      boost::signals2::scoped_connection m_subscription_connection;
      QtPromise<std::vector<Nexus::OrderImbalance>> m_load;
      std::unordered_map<Nexus::Security, Imbalance> m_imbalances;
      QTimer m_expiration_timer;

      std::vector<std::any> make_row(const Nexus::OrderImbalance& imbalance);
      void set_row(int index, const Nexus::OrderImbalance& imbalance);
      void on_expiration_timeout();
      void on_imbalance(const Nexus::OrderImbalance& imbalance);
      void on_load(const std::vector<Nexus::OrderImbalance>& imbalances);
  };
}

#endif
