#ifndef SPIRE_PORTFOLIO_VIEWER_MODEL_HPP
#define SPIRE_PORTFOLIO_VIEWER_MODEL_HPP
#include <unordered_map>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/signals2/signal.hpp>
#include <QAbstractItemModel>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/PortfolioViewer/PortfolioViewer.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /** Models the portfolio's viewed by a manager's account. */
  class PortfolioViewerModel : public QAbstractTableModel {
    public:

      /** Stores a single portfolio entry. */
      struct Entry {

        /** The Entry's account. */
        Beam::DirectoryEntry m_account;

        /** The Entry's group. */
        Beam::DirectoryEntry m_group;

        /** A single Inventory item held by the <i>account</i>. */
        Nexus::Inventory m_inventory;

        /** Constructs an Entry. */
        Entry() = default;

        /**
         * Constructs an Entry.
         * @param account The Entry's account.
         */
        explicit Entry(const Beam::DirectoryEntry& account);
      };

      /** Stores the totals among all displayed Entries. */
      struct TotalEntry {

        /** The total quantity transacted. */
        Nexus::Quantity m_volume;

        /** The total number of transactions. */
        int m_trades;

        /** The unrealized profit and loss. */
        Nexus::Money m_unrealizedProfitAndLoss;

        /** The realized profit and loss. */
        Nexus::Money m_realizedProfitAndLoss;

        /** The total fees paid. */
        Nexus::Money m_fees;

        /** Constructs a TotalEntry. */
        TotalEntry();
      };

      /** The available columns to display. */
      enum Columns {

        /** The group the account belongs to. */
        GROUP_COLUMN,

        /** The account that executed the trades. */
        ACCOUNT_COLUMN,

        /** The account's total profit/loss. */
        ACCOUNT_TOTAL_PROFIT_LOSS_COLUMN,

        /** The account's unrealized profit/loss. */
        ACCOUNT_UNREALIZED_PROFIT_LOSS_COLUMN,

        /** The account's realized profit/loss. */
        ACCOUNT_REALIZED_PROFIT_LOSS_COLUMN,

        /** The total amount of fees paid/received by the account. */
        ACCOUNT_FEES_COLUMN,

        /** The Ticker traded. */
        TICKER_COLUMN,

        /** The position's current quantity. */
        OPEN_QUANTITY_COLUMN,

        /** The position's Side. */
        SIDE_COLUMN,

        /** The average price of the position. */
        AVERAGE_PRICE_COLUMN,

        /** The position's total profit/loss. */
        TOTAL_PROFIT_LOSS_COLUMN,

        /** The position's unrealized profit/loss. */
        UNREALIZED_PROFIT_LOSS_COLUMN,

        /** The position's realized profit/loss. */
        REALIZED_PROFIT_LOSS_COLUMN,

        /** The total fees paid in the position. */
        FEES_COLUMN,

        /** The cost basis. */
        COST_BASIS_COLUMN,

        /** The position's Currency. */
        CURRENCY_COLUMN,

        /** The total quantity traded. */
        VOLUME_COLUMN,

        /** The total number of trades executed. */
        TRADES_COLUMN
      };

      /** The number of columns in this model. */
      static const auto COLUMN_COUNT = 18;

      /**
       * Signals a change to this model's TotalEntry.
       * @param totals The updated TotalEntry.
       */
      using TotalsUpdatedSignal =
        boost::signals2::signal<void (const TotalEntry& entry)>;

      /**
       * Constructs a PorfolioViewerModel.
       * @param userProfile The user's profile.
       * @param selectionModel The PortfolioSelectionModel used to filter this
       *        model.
       */
      PortfolioViewerModel(Beam::Ref<UserProfile> userProfile,
        Beam::Ref<PortfolioSelectionModel> selectionModel);

      /**
       * Returns the Entry at a specified index.
       * @param index The index of the entry to return.
       * @return The Entry at the specified <i>index</i>.
       */
      const Entry& GetEntry(int index) const;

      /** Returns this model's TotalEntry. */
      const TotalEntry& GetTotals() const;

      /**
       * Connects a slot to the TotalsUpdatedSignal.
       * @param slot The slot to connect to the signal.
       * @return A connection to the signal.
       */
      boost::signals2::connection ConnectTotalsUpdatedSignal(
        const TotalsUpdatedSignal::slot_type& slot) const;

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant data(const QModelIndex& index, int role) const override;

      QVariant headerData(
        int section, Qt::Orientation orientation, int role) const override;

    private:
      struct AccountTotals {
        Nexus::Money m_unrealizedProfitAndLoss;
        Nexus::Money m_realizedProfitAndLoss;
        Nexus::Money m_fees;
      };
      struct IndexedEntry : public Entry {
        bool m_isDisplayed;
        int m_displayIndex;
      };
      UserProfile* m_userProfile;
      Nexus::CurrencyId m_totalCurrency;
      const Nexus::ExchangeRateTable* m_exchangeRates;
      PortfolioSelectionModel* m_selectionModel;
      std::vector<IndexedEntry> m_entries;
      int m_displayCount;
      TotalEntry m_totals;
      std::unordered_map<Beam::DirectoryEntry, Beam::DirectoryEntry> m_groups;
      std::unordered_map<Nexus::Ticker, Nexus::Valuation> m_valuations;
      std::unordered_map<Beam::DirectoryEntry, AccountTotals> m_accountTotals;
      std::unordered_map<Nexus::Ticker, std::vector<int>> m_tickerToIndexes;
      std::unordered_map<Nexus::RiskPortfolioKey, int> m_inventoryKeyToIndex;
      EventHandler m_eventHandler;
      mutable TotalsUpdatedSignal m_totalsUpdatedSignal;

      boost::optional<Nexus::Money> GetUnrealizedProfitAndLoss(
        const Nexus::Inventory& inventory) const;
      void OnBboQuote(
        const Nexus::Ticker& ticker, const Nexus::BboQuote& bboQuote);
      void OnRiskPortfolioInventoryUpdate(
        const Nexus::RiskInventoryEntry& entry);
      void OnSelectionModelUpdated(
        const QModelIndex& topLeft, const QModelIndex& bottomRight);
  };
}

#endif
