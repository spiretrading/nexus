#ifndef SPIRE_ORDERIMBALANCEINDICATORMODEL_HPP
#define SPIRE_ORDERIMBALANCEINDICATORMODEL_HPP
#include <map>
#include <optional>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <QAbstractItemModel>
#include <QTimer>
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicator.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorProperties.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class OrderImbalanceIndicatorModel
      \brief Models Market OrderImbalances.
   */
  class OrderImbalanceIndicatorModel : public QAbstractTableModel {
    public:

      /*! \enum Columns
          \brief The available columns to display.
       */
      enum Columns {

        //! The market publishing the OrderImbalance.
        MARKET_COLUMN,

        //! The Security with the OrderImbalance.
        SECURITY_COLUMN,

        //! The Side of the OrderImbalance.
        SIDE_COLUMN,

        //! The size of the OrderImbalance.
        SIZE_COLUMN,

        //! The reference price of the OrderImbalance.
        REFERENCE_PRICE_COLUMN,

        //! The notional value of the OrderImbalance.
        NOTIONAL_VALUE_COLUMN,

        //! The OrderImbalance's time stamp.
        TIMESTAMP_COLUMN
      };

      //! The number of columns in this model.
      static const int COLUMN_COUNT = 7;

      //! Constructs an OrderImbalanceIndicatorModel.
      /*!
        \param userProfile The user's profile.
        \param properties The model's properties.
      */
      OrderImbalanceIndicatorModel(Beam::Ref<UserProfile> userProfile,
        const OrderImbalanceIndicatorProperties& properties);

      //! Returns the properties.
      const OrderImbalanceIndicatorProperties& GetProperties() const;

      //! Specifies whether to filter a market.
      /*!
        \param market The market to specify.
        \param filter <code>true</code> iff the <i>market</i> should be filtered
               out.
      */
      void SetMarketFilter(Nexus::MarketCode market, bool filter);

      //! Updates the end time to query for.
      /*!
        \param startTime The start time to query for.
        \param endTime The end time to query for.
      */
      void UpdateTimeRange(const TimeRangeParameter& startTime,
        const TimeRangeParameter& endTime);

      virtual int rowCount(const QModelIndex& parent) const;

      virtual int columnCount(const QModelIndex& parent) const;

      virtual QVariant data(const QModelIndex& index, int role) const;

      virtual QVariant headerData(int section, Qt::Orientation orientation,
        int role) const;

    private:
      UserProfile* m_userProfile;
      OrderImbalanceIndicatorProperties m_properties;
      QTimer m_updateTimer;
      std::vector<Nexus::MarketOrderImbalance> m_orderImbalances;
      std::vector<Nexus::MarketOrderImbalance> m_displayedOrderImbalances;
      std::map<std::pair<Nexus::MarketCode, Nexus::Security>, int>
        m_imbalanceIndicies;
      std::map<std::pair<Nexus::MarketCode, Nexus::Security>, int>
        m_displayedIndicies;
      std::optional<Beam::TaskQueue> m_slotHandler;

      bool IsDisplayed(const Nexus::MarketOrderImbalance& orderImbalance) const;
      void Reset();
      void InitializePublishers();
      void OnOrderImbalance(Nexus::MarketCode market,
        const Nexus::OrderImbalance& orderImbalance);
      void OnUpdateTimer();
  };
}

#endif
