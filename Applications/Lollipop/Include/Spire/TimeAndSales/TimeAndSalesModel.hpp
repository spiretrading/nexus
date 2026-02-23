#ifndef SPIRE_TIME_AND_SALES_MODEL_HPP
#define SPIRE_TIME_AND_SALES_MODEL_HPP
#include <QAbstractItemModel>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /** Models a ticker's TimeAndSales. */
  class TimeAndSalesModel : public QAbstractTableModel {
    public:

      /** The position of a TimeAndSale print to a BboQuote. */
      enum PriceRange {

        /** The BboQuote was/is not known. */
        UNKNOWN,

        /** The TimeAndSale print was greater than the BBO ask. */
        ABOVE_ASK,

        /** The TimeAndSale print was equal to the BBO ask. */
        AT_ASK,

        /** The TimeAndSale print is inbetween the BBO. */
        INSIDE,

        /** The TimeAndSale print is equal to the BBO bid. */
        AT_BID,

        /** The TimeAndSale print is less than the BBO bid. */
        BELOW_BID
      };

      /** The available columns to display. */
      enum Columns {

        /** The time column. */
        TIME_COLUMN,

        /** The price column. */
        PRICE_COLUMN,

        /** The size column. */
        SIZE_COLUMN,

        /** The market column. */
        MARKET_COLUMN,

        /** The sales condition column. */
        CONDITION_COLUMN,
      };

      /** The number of columns in this model. */
      static const auto COLUMN_COUNT = 5;

      /**
       * Constructs a TimeAndSalesModel.
       * @param userProfile The user's profile.
       * @param properties The TimeAndSalesProperties used.
       * @param ticker The Ticker whose TimeAndSales is to be modeled.
       */
      TimeAndSalesModel(Beam::Ref<UserProfile> userProfile,
        const TimeAndSalesProperties& properties, const Nexus::Ticker& ticker);

      /** Sets the TimeAndSalesProperties. */
      void SetProperties(const TimeAndSalesProperties& properties);

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant data(const QModelIndex& index, int role) const override;

      QVariant headerData(
        int section, Qt::Orientation orientation, int role) const override;

    private:
      UserProfile* m_userProfile;
      TimeAndSalesProperties m_properties;
      Nexus::BboQuote m_bbo;
      std::vector<std::pair<Nexus::TimeAndSale, PriceRange>> m_entries;
      EventHandler m_eventHandler;

      void OnBbo(const Nexus::BboQuote& bbo);
      void OnTimeAndSale(const Nexus::TimeAndSale& timeAndSale);
  };
}

#endif
