#ifndef SPIRE_BOOK_VIEW_MODEL_HPP
#define SPIRE_BOOK_VIEW_MODEL_HPP
#include <memory>
#include <vector>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/Spire/Signal.hpp"

namespace Spire {

  /** Models a security's quotes. */
  class BookViewModel : private boost::noncopyable {
    public:

      /** Indicates the type of quote. */
      enum class Type {

        /** Represents a book quote. */
        BOOK_QUOTE,

        /** Represents a market quote. */
        MARKET_QUOTE,

        /** Represents an order entry. */
        ORDER_ENTRY
      };

      /** Stores a single model quote. */
      struct Quote {

        /** The type of quote. */
        Type m_type;

        /** The book quote representation of the entry. */
        Nexus::BookQuote m_quote;

        /** The entry's price level. */
        int m_price_level;
      };

      //! Signals an update to the BBO.
      using BboSignal = Signal<void (const Nexus::BboQuote& bbo)>;

      //! Signals a price update.
      using PriceSignal = Signal<void (Nexus::Money value)>;

      //! Signals a quantity update.
      using QuantitySignal = Signal<void (Nexus::Quantity value)>;

      //! Signals a quote update.
      using QuoteSignal = Signal<void (const Quote& quote, int index)>;

      virtual ~BookViewModel() = default;

      //! Returns the security being modeled.
      virtual const Nexus::Security& get_security() const = 0;

      //! Returns the BboQuote.
      virtual const Nexus::BboQuote& get_bbo() const = 0;

      //! Returns a snapshot of entries on the ask.
      virtual const std::vector<std::unique_ptr<Quote>>& get_asks() const = 0;

      //! Returns a snapshot of entries on the bid.
      virtual const std::vector<std::unique_ptr<Quote>>& get_bids() const = 0;

      //! Returns the highest price of the session.
      virtual boost::optional<Nexus::Money> get_high() const = 0;

      //! Returns the lowest price of the session.
      virtual boost::optional<Nexus::Money> get_low() const = 0;

      //! Returns the session's opening price.
      virtual boost::optional<Nexus::Money> get_open() const = 0;

      //! Returns the previous session's closing price.
      virtual boost::optional<Nexus::Money> get_close() const = 0;

      //! Returns the session's volume.
      virtual Nexus::Quantity get_volume() const = 0;

      //! Loads the model.
      virtual QtPromise<void> load() = 0;

      //! Returns the market database.
      virtual const Nexus::MarketDatabase& get_market_database() const = 0;

      //! Connects a slot to the bbo signal.
      /*!
        \param slot The slot to connect.
      */
      virtual boost::signals2::connection connect_bbo_slot(
        const BboSignal::slot_type& slot) const = 0;

      //! Connects a slot to the quote signal.
      /*!
        \param slot The slot to connect.
      */
      virtual boost::signals2::connection connect_quote_slot(
        const QuoteSignal::slot_type& slot) const = 0;

      //! Connects a slot to the high signal.
      /*!
        \param slot The slot to connect.
      */
      virtual boost::signals2::connection connect_high_slot(
        const PriceSignal::slot_type& slot) const = 0;

      //! Connects a slot to the low signal.
      /*!
        \param slot The slot to connect.
      */
      virtual boost::signals2::connection connect_low_slot(
        const PriceSignal::slot_type& slot) const = 0;

      //! Connects a slot to the open signal.
      /*!
        \param slot The slot to connect.
      */
      virtual boost::signals2::connection connect_open_slot(
        const PriceSignal::slot_type& slot) const = 0;

      //! Connects a slot to the close signal.
      /*!
        \param slot The slot to connect.
      */
      virtual boost::signals2::connection connect_close_slot(
        const PriceSignal::slot_type& slot) const = 0;

      //! Connects a slot to the volume signal.
      /*!
        \param slot The slot to connect.
      */
      virtual boost::signals2::connection connect_volume_slot(
        const QuantitySignal::slot_type& slot) const = 0;

    protected:

      //! Constructs a book view model.
      BookViewModel() = default;
  };
}

#endif
