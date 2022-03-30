#ifndef SPIRE_LOCAL_ORDER_IMBALANCE_INDICATOR_TABLE_MODEL_HPP
#define SPIRE_LOCAL_ORDER_IMBALANCE_INDICATOR_TABLE_MODEL_HPP
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Spire/Ui/ArrayTableModel.hpp"

namespace Spire {

  /** Implements a TableModel for OrderImbalances in memory. */
  class LocalOrderImbalanceIndicatorTableModel : public TableModel {
    public:

      /**
       * Appends the given OrderImbalance to the table if an imbalance with the
       * same security is not already in the table, or, updates the existing
       * imbalance iff the given imbalance is newer than the existing imbalance
       * for the same security.
       * @param imbalance The imbalance to add to the model.
       */
      void add(const Nexus::OrderImbalance& imbalance);

      /**
       * Removes any OrderImbalance associated with the given security.
       * @param security The security for the OrderImbalance to remove.
       */
      void remove(const Nexus::Security& security);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      struct MappedImbalance {
        int m_row_index;
        Nexus::OrderImbalance m_imbalance;
      };
      ArrayTableModel m_table;
      std::unordered_map<Nexus::Security, MappedImbalance> m_imbalances;

      void set_row(
        const Nexus::OrderImbalance& current, MappedImbalance& previous);
      std::vector<std::any> make_row(
        const Nexus::OrderImbalance& imbalance) const;
      std::vector<boost::optional<std::any>>
        make_row_update(const Nexus::OrderImbalance& current,
          const Nexus::OrderImbalance& previous) const;
  };
}

#endif
