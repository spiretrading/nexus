#ifndef LOCAL_ORDER_IMBALANCE_INDICATOR_TABLE_MODEL_HPP
#define LOCAL_ORDER_IMBALANCE_INDICATOR_TABLE_MODEL_HPP
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Spire/Ui/ArrayTableModel.hpp"

namespace Spire {

  /** Implements a TableModel for OrderImbalances in memory. */
  class LocalOrderImbalanceIndicatorTableModel : public TableModel {
    public:

      /**
       * Appends the given OrderImbalance to the table, or, updates the
       * existing imbalance if the associated security has an older
       * imbalance in the table.
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

      const std::any& at(int row, int column) const override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

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
      std::vector<boost::optional<std::any>> make_row_update(
        const OrderImbalance& current, const OrderImbalance& previous) const;
  };
}

#endif
