#ifndef SPIRE_BOOKVIEWMODEL_HPP
#define SPIRE_BOOKVIEWMODEL_HPP
#include <map>
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <QAbstractItemModel>
#include <QTimer>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class BookViewModel
      \brief Models a single Side of a Security's book.
   */
  class BookViewModel : public QAbstractTableModel {
    public:

      /*! \enum Columns
          \brief Enumerates the model's columns.
       */
      enum Columns {

        //! The MPID column.
        MPID_COLUMN,

        //! The price column.
        PRICE_COLUMN,

        //! The size column.
        SIZE_COLUMN,
      };

      //! The number of columns available.
      static const unsigned int COLUMN_COUNT = 3;

      //! Constructs a BookViewModel.
      /*!
        \param userProfile The user's profile.
        \param properties The BookViewProperties used to display the Quotes.
        \param security The Security whose book is to be modeled.
        \param side The Side of the book to model.
      */
      BookViewModel(Beam::Ref<UserProfile> userProfile,
        const BookViewProperties& properties, const Nexus::Security& security,
        Nexus::Side side);

      virtual ~BookViewModel();

      /** Returns the SecurityInfo this model represents. */
      const Nexus::SecurityInfo& GetSecurityInfo() const;

      //! Sets the properties.
      /*!
        \param properties The display properties.
      */
      void SetProperties(const BookViewProperties& properties);

      int rowCount(const QModelIndex& parent) const;

      int columnCount(const QModelIndex& parent) const;

      QVariant data(const QModelIndex& index, int role) const;

      QVariant headerData(int section, Qt::Orientation orientation,
        int role) const;

    private:
      struct OrderKey {
        Nexus::Money m_price;
        std::string m_destination;

        bool operator <(const OrderKey& value) const;
      };
      struct BookQuoteEntry {
        Nexus::BookQuote m_quote;
        int m_level;
      };
      UserProfile* m_userProfile;
      BookViewProperties m_properties;
      Nexus::Security m_security;
      Nexus::Side m_side;
      Nexus::SecurityInfo m_securityInfo;
      std::unordered_map<Nexus::MarketCode, Nexus::MarketQuote> m_marketQuotes;
      std::unordered_map<Nexus::MarketCode, Nexus::BookQuote> m_topLevels;
      std::vector<std::unique_ptr<BookQuoteEntry>> m_bookQuotes;
      std::map<OrderKey, Nexus::Quantity> m_orderQuantities;
      std::unordered_map<const Nexus::OrderExecutionService::Order*,
        Nexus::Quantity> m_remainingOrderQuantities;
      QTimer m_updateTimer;
      std::shared_ptr<Beam::TaskQueue> m_slotHandler;

      bool TestHighlight(const BookViewProperties::MarketHighlight& highlight,
        const Nexus::BookQuote& quote) const;
      void HighlightQuote(const Nexus::BookQuote& quote);
      void AddQuote(const Nexus::BookQuote& quote, int quoteIndex);
      void RemoveQuote(int quoteIndex);
      void OnMarketQuote(const Nexus::MarketQuote& quote);
      void OnBookQuote(const Nexus::BookQuote& quote);
      void OnOrderExecuted(const Nexus::OrderExecutionService::Order* order);
      void OnExecutionReport(const Nexus::OrderExecutionService::Order* order,
        const Nexus::OrderExecutionService::ExecutionReport& executionReport);
      void OnBookQuoteInterruption(const std::exception_ptr& e);
      void OnMarketQuoteInterruption(const std::exception_ptr& e);
      void OnUpdateTimer();
  };
}

#endif
