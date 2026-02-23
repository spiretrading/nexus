#ifndef SPIRE_OPEN_POSITIONS_MODEL_HPP
#define SPIRE_OPEN_POSITIONS_MODEL_HPP
#include <optional>
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <QAbstractItemModel>
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/ProfitAndLossModel.hpp"

namespace Spire {

  /** Models a blotter's open positions. */
  class OpenPositionsModel : public QAbstractTableModel {
    public:

      /** Enumerates the columns displayed in the open positions table. */
      enum Columns {

        /** The Ticker. */
        TICKER_COLUMN,

        /** The position's quantity. */
        QUANTITY_COLUMN,

        /** The position's Side. */
        SIDE_COLUMN,

        /** The average price of the position. */
        AVERAGE_PRICE_COLUMN,

        /** The position's profit/loss. */
        PROFIT_LOSS_COLUMN,

        /** The cost basis. */
        COST_BASIS_COLUMN,

        /** The position's Currency. */
        CURRENCY_COLUMN
      };

      /** The number of columns available. */
      static const auto COLUMNS = 7;

      /** Stores an entry for a single open position. */
      struct Entry {

        /** The Entry's index. */
        int m_index;

        /** The open position's ticker. */
        Nexus::Ticker m_ticker;

        /** The Inventory representing the open position. */
        Nexus::Inventory m_inventory;

        /** The position's unrealized earnings. */
        Nexus::Money m_unrealizedEarnings;

        /**
         * Constructs an Entry.
         * @param index The Entry's index.
         * @param ticker The open position's ticker.
         */
        Entry(int index, const Nexus::Ticker& ticker);
      };

      /** Constructs an empty OpenPositionsModel. */
      OpenPositionsModel();

      /**
       * Sets the PortfolioController whose open positions are to be modeled.
       * @param portfolioController The PortfolioController whose open positions
       *        are to be modelled.
       */
      void SetPortfolioController(
        Beam::Ref<ProfitAndLossModel::PortfolioController> portfolioController);

      /** Returns all open positions. */
      std::vector<Entry> GetOpenPositions() const;

      /**
       * Returns the Entry for an open position.
       * @param ticker The Ticker whose open position Entry is to be
       *        retrieved.
       * @return The Entry for the <i>ticker</i>'s open position iff it
       *         exists.
       */
      boost::optional<const Entry&>
        GetOpenPosition(const Nexus::Ticker& ticker) const;

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant data(const QModelIndex& index, int role) const override;

      QVariant headerData(
        int section, Qt::Orientation orientation, int role) const override;

    private:
      ProfitAndLossModel::PortfolioController* m_portfolioController;
      std::vector<std::unique_ptr<Entry>> m_entries;
      std::unordered_map<Nexus::Ticker, Entry*> m_tickerToEntry;
      std::optional<EventHandler> m_eventHandler;

      void OnPortfolioUpdate(const Nexus::PortfolioUpdateEntry& update);
  };
}

#endif
