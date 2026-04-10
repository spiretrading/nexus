#ifndef SPIRE_BOOK_VIEW_MODEL_HPP
#define SPIRE_BOOK_VIEW_MODEL_HPP
#include <map>
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <QAbstractItemModel>
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/TickerInfo.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/BookView/BookViewProperties.hpp"

namespace Spire {

  /** Models a single Side of a Ticker's book. */
  class BookViewModel : public QAbstractTableModel {
    public:

      /** Enumerates the model's columns. */
      enum Columns {

        /** The MPID column. */
        MPID_COLUMN,

        /** The price column. */
        PRICE_COLUMN,

        /** The size column. */
        SIZE_COLUMN,
      };

      /** The number of columns available. */
      static const auto COLUMN_COUNT = 3;

      /**
       * Constructs a BookViewModel.
       * @param userProfile The user's profile.
       * @param properties The BookViewProperties used to display the Quotes.
       * @param ticker The Ticker whose book is to be modeled.
       * @param side The Side of the book to model.
       */
      BookViewModel(Beam::Ref<UserProfile> userProfile,
        const BookViewProperties& properties, const Nexus::Ticker& ticker,
        Nexus::Side side);

      /**
       * Sets the properties.
       * @param properties The display properties.
       */
      void SetProperties(const BookViewProperties& properties);

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant data(const QModelIndex& index, int role) const override;

      QVariant headerData(
        int section, Qt::Orientation orientation, int role) const override;

    private:
      struct OrderKey {
        Nexus::Money m_price;
        std::string m_destination;

        auto operator <=>(const OrderKey&) const = default;
      };
      struct BookQuoteEntry {
        Nexus::BookQuote m_quote;
        int m_level;
      };
      UserProfile* m_userProfile;
      BookViewProperties m_properties;
      Nexus::Ticker m_ticker;
      Nexus::Side m_side;
      Nexus::TickerInfo m_tickerInfo;
      std::unordered_map<Nexus::Venue, Nexus::BookQuote> m_topLevels;
      std::vector<std::unique_ptr<BookQuoteEntry>> m_bookQuotes;
      std::map<OrderKey, Nexus::Quantity> m_orderQuantities;
      std::unordered_map<std::shared_ptr<Nexus::Order>, Nexus::Quantity>
        m_remainingOrderQuantities;
      EventHandler m_eventHandler;

      bool TestHighlight(const BookViewProperties::VenueHighlight& highlight,
        const Nexus::BookQuote& quote) const;
      void HighlightQuote(const Nexus::BookQuote& quote);
      void AddQuote(const Nexus::BookQuote& quote, int quoteIndex);
      void RemoveQuote(int quoteIndex);
      void OnBookQuote(const Nexus::BookQuote& quote);
      void OnOrderExecuted(const std::shared_ptr<Nexus::Order>& order);
      void OnExecutionReport(const std::shared_ptr<Nexus::Order>& order,
        const Nexus::ExecutionReport& executionReport);
      void OnBookQuoteInterruption(const std::exception_ptr& e);
  };
}

#endif
