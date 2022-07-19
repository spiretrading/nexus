#ifndef SPIRE_BLOTTER_PROFIT_AND_LOSS_MODEL_HPP
#define SPIRE_BLOTTER_PROFIT_AND_LOSS_MODEL_HPP
#include <boost/variant/variant.hpp>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/PortfolioModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"

namespace Spire {

  /**
   * Transforms the profit and losses in a PortfolioModel into a TableModel
   * where each individual security/currency is represented as a row.
   */
  class BlotterProfitAndLossModel : public TableModel {
    public:

      /** Stores a currency index and whether that currency is expanded. */
      struct CurrencyIndex {

        /** The currency being indexed. */
        Nexus::CurrencyId m_index;

        /** Whether the currency is expanded. */
        bool m_is_expanded;
      };

      /** Lists out the columns of the table. */
      enum Column {

        /** The profit and loss's currency. */
        CURRENCY,

        /** The security that was traded in the respective currency. */
        SECURITY,

        /** The profit and loss for the respective currency/security. */
        PROFIT_AND_LOSS,

        /** The total fees paid. */
        FEES,

        /** The total shares traded. */
        VOLUME
      };

      /** The number of columns in this table. */
      static const auto COLUMN_SIZE = 5;

      /**
       * Constructs a BlotterProfitAndLossModel representing a PortfolioModel.
       * @param portfolio The portfolio to represent.
       */
      explicit BlotterProfitAndLossModel(
        std::shared_ptr<PortfolioModel> portfolio);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      using Index = boost::variant<Nexus::Security, Nexus::CurrencyId>;
      std::shared_ptr<PortfolioModel> m_portfolio;
      std::unordered_map<Index, int> m_indexes;
      ArrayTableModel m_table;
      boost::signals2::scoped_connection m_connection;

      void update(const Index& index, Nexus::Money unrealized_profit_and_loss,
        const PortfolioModel::Portfolio::UpdateEntry::Inventory& inventory);
      void on_update(const PortfolioModel::Portfolio::UpdateEntry& entry);
  };
}

#endif
