#ifndef SPIRE_TICKER_INFO_MODEL_HPP
#define SPIRE_TICKER_INFO_MODEL_HPP
#include <unordered_set>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <QAbstractTableModel>
#include "Nexus/Definitions/TickerInfo.hpp"
#include "Spire/Async/QtPromise.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /** Models the TickerInfo's available on the MarketDataServer. */
  class TickerInfoModel : public QAbstractTableModel {
    public:

      /** Enumerates the model's columns. */
      enum Columns {

        /** The Ticker column. */
        TICKER_COLUMN,

        /** The name column. */
        NAME_COLUMN,

        /** The sector column. */
        SECTOR_COLUMN
      };

      /** The number of columns available. */
      static const auto COLUMN_COUNT = 3;

      /**
       * Constructs a TickerInfoModel.
       * @param userProfile The user's profile.
       */
      explicit TickerInfoModel(Beam::Ref<UserProfile> userProfile);

      /**
       * Searches for TickerInfo objects matching a prefix.
       * @param prefix The prefix to search for.
       */
      void Search(const std::string& prefix);

      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& parent) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      QVariant headerData(
        int section, Qt::Orientation orientation, int role) const override;

    private:
      UserProfile* m_userProfile;
      std::vector<Nexus::TickerInfo> m_tickerInfoItems;
      std::unordered_set<std::string> m_prefixes;
      QtPromise<void> m_queryPromise;

      void AddTickerInfoItems(std::vector<Nexus::TickerInfo> tickerInfoItems);
  };
}

#endif
