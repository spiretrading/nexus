#ifndef SPIRE_TIMEANDSALESMODEL_HPP
#define SPIRE_TIMEANDSALESMODEL_HPP
#include <Beam/Queues/TaskQueue.hpp>
#include <QAbstractItemModel>
#include <QTimer>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"

namespace Spire {

  /*! \class TimeAndSalesModel
      \brief Models a Security's TimeAndSales.
   */
  class TimeAndSalesModel : public QAbstractTableModel {
    public:

      /*! \enum PriceRange
          \brief The position of a TimeAndSale print to a BboQuote.
       */
      enum PriceRange {

        //! The BboQuote was/is not known.
        UNKNOWN,

        //! The TimeAndSale print was greater than the BBO ask.
        ABOVE_ASK,

        //! The TimeAndSale print was equal to the BBO ask.
        AT_ASK,

        //! The TimeAndSale print is inbetween the BBO.
        INSIDE,

        //! The TimeAndSale print is equal to the BBO bid.
        AT_BID,

        //! The TimeAndSale print is less than the BBO bid.
        BELOW_BID,
      };

      /*! \enum Columns
       *  \brief The available columns to display.
       */
      enum Columns {

        //! The time column.
        TIME_COLUMN,

        //! The price column.
        PRICE_COLUMN,

        //! The size column.
        SIZE_COLUMN,

        //! The market column.
        MARKET_COLUMN,

        //! The sales condition column.
        CONDITION_COLUMN,
      };

      //! The number of columns in this model.
      static const int COLUMN_COUNT = 5;

      //! Constructs a TimeAndSalesModel.
      /*!
        \param userProfile The user's profile.
        \param properties The TimeAndSalesProperties used.
        \param security The Security whose TimeAndSales is to be modeled.
      */
      TimeAndSalesModel(Beam::Ref<UserProfile> userProfile,
        const TimeAndSalesProperties& properties,
        const Nexus::Security& security);

      //! Sets the TimeAndSalesProperties.
      void SetProperties(const TimeAndSalesProperties& properties);

      virtual int rowCount(const QModelIndex& parent) const;

      virtual int columnCount(const QModelIndex& parent) const;

      virtual QVariant data(const QModelIndex& index, int role) const;

      virtual QVariant headerData(int section, Qt::Orientation orientation,
        int role) const;

    private:
      UserProfile* m_userProfile;
      TimeAndSalesProperties m_properties;
      Nexus::BboQuote m_bbo;
      std::vector<std::pair<Nexus::TimeAndSale, PriceRange>> m_entries;
      QTimer m_updateTimer;
      Beam::TaskQueue m_slotHandler;

      void OnBbo(const Nexus::BboQuote& bbo);
      void OnTimeAndSale(const Nexus::TimeAndSale& timeAndSale);
      void OnUpdateTimer();
  };
}

#endif
