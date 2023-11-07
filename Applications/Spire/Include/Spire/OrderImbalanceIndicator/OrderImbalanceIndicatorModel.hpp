#ifndef SPIRE_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include <map>
#include <optional>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <QAbstractItemModel>
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorProperties.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Models Market OrderImbalances. */
  class OrderImbalanceIndicatorModel : public QAbstractTableModel {
    public:

      /** The available columns to display. */
      enum Columns {

        /** The market publishing the OrderImbalance. */
        MARKET_COLUMN,

        /** The Security with the OrderImbalance. */
        SECURITY_COLUMN,

        /** The Side of the OrderImbalance. */
        SIDE_COLUMN,

        /** The size of the OrderImbalance. */
        SIZE_COLUMN,

        /** The reference price of the OrderImbalance. */
        REFERENCE_PRICE_COLUMN,

        /** The notional value of the OrderImbalance. */
        NOTIONAL_VALUE_COLUMN,

        /** The OrderImbalance's time stamp. */
        TIMESTAMP_COLUMN
      };

      /** The number of columns in this model. */
      static const auto COLUMN_COUNT = 7;

      /**
       * Constructs an OrderImbalanceIndicatorModel.
       * @param userProfile The user's profile.
       * @param properties The model's properties.
       */
      OrderImbalanceIndicatorModel(Beam::Ref<UserProfile> userProfile,
        const OrderImbalanceIndicatorProperties& properties);

      /** Returns the properties. */
      const OrderImbalanceIndicatorProperties& GetProperties() const;

      /**
       * Specifies whether to filter a market.
       * @param market The market to specify.
       * @param filter <code>true</code> iff the <i>market</i> should be
       *        filtered out.
       */
      void SetMarketFilter(Nexus::MarketCode market, bool filter);

      /**
       * Updates the end time to query for.
       * @param startTime The start time to query for.
       * @param endTime The end time to query for.
       */
      void UpdateTimeRange(
        const TimeRangeParameter& startTime, const TimeRangeParameter& endTime);

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant data(const QModelIndex& index, int role) const override;

      QVariant headerData(
        int section, Qt::Orientation orientation, int role) const override;

    private:
      UserProfile* m_userProfile;
      OrderImbalanceIndicatorProperties m_properties;
      std::vector<Nexus::MarketOrderImbalance> m_orderImbalances;
      std::vector<Nexus::MarketOrderImbalance> m_displayedOrderImbalances;
      std::map<std::pair<Nexus::MarketCode, Nexus::Security>, int>
        m_imbalanceIndicies;
      std::map<std::pair<Nexus::MarketCode, Nexus::Security>, int>
        m_displayedIndicies;
      std::optional<EventHandler> m_eventHandler;

      bool IsDisplayed(const Nexus::MarketOrderImbalance& orderImbalance) const;
      void Reset();
      void InitializePublishers();
      void OnOrderImbalance(
        Nexus::MarketCode market, const Nexus::OrderImbalance& orderImbalance);
  };
}

#endif
