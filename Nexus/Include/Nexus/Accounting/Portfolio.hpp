#ifndef NEXUS_PORTFOLIO_HPP
#define NEXUS_PORTFOLIO_HPP
#include <type_traits>
#include <unordered_map>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Accounting/Bookkeeper.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores an update to a Portfolio's snapshot. */
  struct PortfolioUpdateEntry {

    /** The updated Inventory. */
    Inventory m_inventory;

    /** The updated unrealized gross earnings. */
    Money m_unrealized;

    /** The updated currency Inventory. */
    Inventory m_currency_inventory;

    /** The updated unrealized gross earnings for the currency. */
    Money m_unrealized_currency;
  };

  /** Stores a valuation. */
  struct Valuation {

    /** The currency used to denominate the value. */
    Asset m_currency;

    /** The ask side value. */
    boost::optional<Money> m_ask_value;

    /** The bid side value. */
    boost::optional<Money> m_bid_value;
  };

  /**
   * Monitors transactions as part of a portfolio and reports the P/L.
   * @param <B> The type of Bookkeeper to use.
   */
  template<IsBookkeeper B>
  class Portfolio {
    public:

      /** The type of Bookkeeper to use. */
      using Bookkeeper = B;

      /** Stores portfolio related info about a single entry. */
      struct Entry {

        /** The current value of the entry. */
        Valuation m_valuation;

        /** The unrealized profit and loss. */
        Money m_unrealized;

        /**
         * Constructs an empty Entry with no valuation or unrealized profit and
         * loss.
         */
        Entry() = default;

        /**
         * Constructs an Entry.
         * @param currency The currency used to value the entry.
         */
        explicit Entry(Asset currency) noexcept;

        /**
         * Constructs an Entry.
         * @param valuation The valuation to use for the entry.
         * @param unrealized The unrealized profit and loss for the entry.
         */
        Entry(Valuation valuation, Money unrealized) noexcept;
      };

      /** The type used to map tickers to entries. */
      using EntryMap = std::unordered_map<Ticker, Entry>;

      /** The type used to map currencies to unrealized profit and losses. */
      using UnrealizedProfitAndLossMap = std::unordered_map<Asset, Money>;

      /** Constructs an empty Portfolio. */
      Portfolio() = default;

      /**
       * Constructs a Portfolio for an existing set of positions.
       * @param bookkeeper The bookkeeper with the existing set of positions.
       */
      explicit Portfolio(Bookkeeper bookkeeper);

      /** Returns the Bookkeeper. */
      const Bookkeeper& get_bookkeeper() const;

      /** Returns the Portfolio's entries. */
      const EntryMap& get_entries() const;

      /** Returns the Portfolio's unrealized profit and losses. */
      const UnrealizedProfitAndLossMap&
        get_unrealized_profit_and_losses() const;

      /**
       * Updates a position held by this Portfolio.
       * @param fields Specifies the Order's details.
       * @param report The ExecutionReport specifying the update.
       * @return <code>true</code> iff the update resulted in a change to
       *         the portfolio.
       */
      bool update(const OrderFields& fields, const ExecutionReport& report);

      /**
       * Updates a ticker's ask value.
       * @param ticker The ticker whose value is being updated.
       * @param value The <i>ticker</i>'s ask side value.
       * @return <code>true</code> iff the update resulted in a change to
       *         the portfolio.
       */
      bool update_ask(const Ticker& ticker, Money value);

      /**
       * Updates a ticker's bid value.
       * @param ticker The ticker whose value is being updated.
       * @param value The <i>ticker</i>'s bid side value.
       * @return <code>true</code> iff the update resulted in a change to
       *         the portfolio.
       */
      bool update_bid(const Ticker& ticker, Money value);

      /**
       * Updates the market value of a ticker.
       * @param ticker The ticker whose value is being updated.
       * @param ask_value The <i>ticker</i>'s ask side value.
       * @param bid_value The <i>ticker</i>'s bid side value.
       * @return <code>true</code> iff the update resulted in a change to
       *         the portfolio.
       */
      bool update(const Ticker& ticker, Money ask_value, Money bid_value);

    private:
      Bookkeeper m_bookkeeper;
      EntryMap m_entries;
      UnrealizedProfitAndLossMap m_unrealized_currencies;

      static boost::optional<Money> calculate_unrealized(
        const Inventory& inventory, const Entry& entry);
      Entry& get_entry(const Ticker& ticker, Asset currency);
      bool update(const Ticker& ticker, Entry& entry);
  };

  /**
   * Returns the realized profit and loss of a single Inventory.
   * @param inventory The Inventory to calculate the realized profit and loss
   *        of.
   * @return The <i>inventory</i>'s profit and loss.
   */
  inline Money get_realized_profit_and_loss(const Inventory& inventory) {
    return inventory.m_gross_profit_and_loss - inventory.m_fees;
  }

  /**
   * Returns the unrealized profit and loss of a single Inventory.
   * @param inventory The Inventory to calculate the unrealized profit and loss
   *        of.
   * @param valuation The valuation to use on the <i>inventory</i>.
   * @return The <i>inventory</i>'s unrealized profit and loss.
   */
  inline boost::optional<Money> get_unrealized_profit_and_loss(
      const Inventory& inventory, const Valuation& valuation) {
    if(inventory.m_position.m_quantity == 0) {
      return Money::ZERO;
    }
    auto price = Money();
    if(inventory.m_position.m_quantity > 0) {
      if(valuation.m_bid_value) {
        price = *valuation.m_bid_value;
      } else {
        return boost::none;
      }
    } else {
      if(valuation.m_ask_value) {
        price = *valuation.m_ask_value;
      } else {
        return boost::none;
      }
    }
    return inventory.m_position.m_quantity * price -
      inventory.m_position.m_cost_basis;
  }

  /**
   * Returns the total (realized and unrealized) profit and loss of a single
   * Inventory.
   * @param inventory The Inventory to calculate the total profit and loss of.
   * @param valuation The valuation to use on the <i>inventory</i>.
   * @return The <i>inventory</i>'s total profit and loss.
   */
  inline boost::optional<Money> get_total_profit_and_loss(
      const Inventory& inventory, const Valuation& valuation) {
    if(auto unrealized_profit_and_loss =
        get_unrealized_profit_and_loss(inventory, valuation)) {
      return get_realized_profit_and_loss(inventory) +
        *unrealized_profit_and_loss;
    }
    return boost::none;
  }

  /**
   * Returns the total (realized and unrealized) profit and loss of all
   * Inventory in a given currency within a Portfolio.
   * @param portfolio The Portfolio to calculate the total profit and loss for.
   * @param currency The currency to calculate the total profit and loss for.
   * @return The total profit and loss of  all Inventory in the <i>portfolio</i>
   *         for the specified <i>currency</i>.
   */
  template<typename B>
  Money get_total_profit_and_loss(
      const Portfolio<B>& portfolio, Asset currency) {
    auto unrealized_profit_and_loss = Money();
    auto profit_and_loss_iterator =
      portfolio.get_unrealized_profit_and_losses().find(currency);
    if(profit_and_loss_iterator ==
        portfolio.get_unrealized_profit_and_losses().end()) {
      unrealized_profit_and_loss = Money::ZERO;
    } else {
      unrealized_profit_and_loss = profit_and_loss_iterator->second;
    }
    auto& inventory = portfolio.get_bookkeeper().get_total(currency);
    return unrealized_profit_and_loss + get_realized_profit_and_loss(inventory);
  }

  /**
   * Calls a function for each entry in a Portfolio.
   * @param portfolio The Portfolio to get entries from.
   * @param f The function to call with a Portfolio's entry.
   */
  template<typename B, typename F>
    requires std::invocable<F, PortfolioUpdateEntry>
  void for_each(const Portfolio<B>& portfolio, F f) {
    auto& entries = portfolio.get_entries();
    for(auto& [ticker, entry] : entries) {
      auto update = PortfolioUpdateEntry();
      update.m_inventory = portfolio.get_bookkeeper().get_inventory(ticker);
      if(is_empty(update.m_inventory)) {
        continue;
      }
      update.m_unrealized = entry.m_unrealized;
      update.m_currency_inventory = portfolio.get_bookkeeper().get_total(
        entry.m_valuation.m_currency);
      auto unrealized_currency_iterator =
        portfolio.get_unrealized_profit_and_losses().find(
          entry.m_valuation.m_currency);
      if(unrealized_currency_iterator ==
          portfolio.get_unrealized_profit_and_losses().end()) {
        update.m_unrealized_currency = Money::ZERO;
      } else {
        update.m_unrealized_currency = unrealized_currency_iterator->second;
      }
      f(std::move(update));
    }
  }

  inline std::ostream& operator <<(
      std::ostream& out, const PortfolioUpdateEntry& entry) {
    return out << '(' << entry.m_inventory << ' ' << entry.m_unrealized <<
      ' ' << entry.m_currency_inventory << ' ' << entry.m_unrealized_currency <<
      ')';
  }

  inline std::ostream& operator <<(
      std::ostream& out, const Valuation& valuation) {
    out << '(' << valuation.m_currency << ' ';
    if(valuation.m_ask_value) {
      out << *valuation.m_ask_value;
    } else {
      out << "None";
    }
    out << ' ';
    if(valuation.m_bid_value) {
      out << *valuation.m_bid_value;
    } else {
      out << "None";
    }
    out << ')';
    return out;
  }

  template<IsBookkeeper B>
  Portfolio<B>::Entry::Entry(Asset currency) noexcept
    : m_valuation(currency) {}

  template<IsBookkeeper B>
  Portfolio<B>::Entry::Entry(Valuation valuation, Money unrealized) noexcept
    : m_valuation(std::move(valuation)),
      m_unrealized(unrealized) {}

  template<IsBookkeeper B>
  Portfolio<B>::Portfolio(Bookkeeper bookkeeper)
      : m_bookkeeper(std::move(bookkeeper)) {
    for(auto& inventory : m_bookkeeper.get_inventory_range()) {
      get_entry(inventory.m_position.m_ticker, inventory.m_position.m_currency);
    }
  }

  template<IsBookkeeper B>
  const typename Portfolio<B>::Bookkeeper&
      Portfolio<B>::get_bookkeeper() const {
    return m_bookkeeper;
  }

  template<IsBookkeeper B>
  const typename Portfolio<B>::EntryMap& Portfolio<B>::get_entries() const {
    return m_entries;
  }

  template<IsBookkeeper B>
  const typename Portfolio<B>::UnrealizedProfitAndLossMap&
      Portfolio<B>::get_unrealized_profit_and_losses() const {
    return m_unrealized_currencies;
  }

  template<IsBookkeeper B>
  bool Portfolio<B>::update(
      const OrderFields& fields, const ExecutionReport& report) {
    if(report.m_last_quantity == 0) {
      return false;
    }
    auto& ticker = fields.m_ticker;
    auto currency = fields.m_currency;
    auto& entry = get_entry(ticker, currency);
    auto quantity = get_direction(fields.m_side) * report.m_last_quantity;
    m_bookkeeper.record(ticker, currency, quantity,
      report.m_last_quantity * report.m_last_price, get_fee_total(report));
    auto inventory = m_bookkeeper.get_inventory(ticker);
    if(auto unrealized = calculate_unrealized(inventory, entry)) {
      auto& unrealized_currency = m_unrealized_currencies[currency];
      if(*unrealized != entry.m_unrealized) {
        unrealized_currency -= entry.m_unrealized;
        entry.m_unrealized = *unrealized;
        unrealized_currency += entry.m_unrealized;
      }
    }
    return true;
  }

  template<IsBookkeeper B>
  bool Portfolio<B>::update_ask(const Ticker& ticker, Money value) {
    auto& entry = get_entry(ticker, Asset());
    entry.m_valuation.m_ask_value = value;
    return update(ticker, entry);
  }

  template<IsBookkeeper B>
  bool Portfolio<B>::update_bid(const Ticker& ticker, Money value) {
    auto& entry = get_entry(ticker, Asset());
    entry.m_valuation.m_bid_value = value;
    return update(ticker, entry);
  }

  template<IsBookkeeper B>
  bool Portfolio<B>::update(
      const Ticker& ticker, Money ask_value, Money bid_value) {
    auto& entry = get_entry(ticker, Asset());
    entry.m_valuation.m_ask_value = ask_value;
    entry.m_valuation.m_bid_value = bid_value;
    return update(ticker, entry);
  }

  template<IsBookkeeper B>
  boost::optional<Money> Portfolio<B>::calculate_unrealized(
      const Inventory& inventory, const Entry& entry) {
    auto valuation = [&] {
      if(inventory.m_position.m_quantity >= 0) {
        return entry.m_valuation.m_bid_value;
      }
      return entry.m_valuation.m_ask_value;
    }();
    if(valuation) {
      return inventory.m_position.m_quantity * *valuation -
        inventory.m_position.m_cost_basis;
    }
    return boost::none;
  }

  template<IsBookkeeper B>
  typename Portfolio<B>::Entry& Portfolio<B>::get_entry(
      const Ticker& ticker, Asset currency) {
    auto entry_iterator = m_entries.find(ticker);
    if(entry_iterator == m_entries.end()) {
      entry_iterator =
        m_entries.insert(std::pair(ticker, Entry(currency))).first;
    } else if(!entry_iterator->second.m_valuation.m_currency) {
      entry_iterator->second.m_valuation.m_currency = currency;
    }
    return entry_iterator->second;
  }

  template<IsBookkeeper B>
  bool Portfolio<B>::update(const Ticker& ticker, Entry& entry) {
    auto& inventory = m_bookkeeper.get_inventory(ticker);
    auto unrealized = calculate_unrealized(inventory, entry);
    if(!unrealized || *unrealized == entry.m_unrealized) {
      return false;
    }
    auto& unrealized_currency =
      m_unrealized_currencies[entry.m_valuation.m_currency];
    unrealized_currency -= entry.m_unrealized;
    entry.m_unrealized = *unrealized;
    unrealized_currency += entry.m_unrealized;
    return true;
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::PortfolioUpdateEntry> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::PortfolioUpdateEntry& value,
        unsigned int version) const {
      shuttle.shuttle("inventory", value.m_inventory);
      shuttle.shuttle("unrealized", value.m_unrealized);
      shuttle.shuttle("currency_inventory", value.m_currency_inventory);
      shuttle.shuttle("unrealized_currency", value.m_unrealized_currency);
    }
  };

  template<>
  struct Shuttle<Nexus::Valuation> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::Valuation& value, unsigned int version) const {
      shuttle.shuttle("currency", value.m_currency);
      shuttle.shuttle("ask_value", value.m_ask_value);
      shuttle.shuttle("bid_value", value.m_bid_value);
    }
  };
}

#endif
