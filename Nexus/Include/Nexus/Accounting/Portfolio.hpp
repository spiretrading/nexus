#ifndef NEXUS_PORTFOLIO_HPP
#define NEXUS_PORTFOLIO_HPP
#include <type_traits>
#include <unordered_map>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Accounting/Bookkeeper.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Venue.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /** Stores an update to a Portfolio's snapshot. */
  struct PortfolioUpdateEntry {

    /** The updated Security Inventory. */
    Inventory m_security_inventory;

    /** The updated unrealized gross earnings for the Security. */
    Money m_unrealized_security;

    /** The updated Currency Inventory. */
    Inventory m_currency_inventory;

    /** The updated unrealized gross earnings for the Currency. */
    Money m_unrealized_currency;
  };

  /** Stores the valuation used for a Security. */
  struct SecurityValuation {

    /** The currency used to value the Security. */
    CurrencyId m_currency;

    /** The ask side value. */
    boost::optional<Money> m_ask_value;

    /** The bid side value. */
    boost::optional<Money> m_bid_value;

    /** Constructs a SecurityValuation. */
    SecurityValuation() = default;

    /**
     * Constructs a SecurityValuation.
     * @param currency The currency used to value the Security.
     */
    explicit SecurityValuation(CurrencyId currency) noexcept;
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

      /** Stores portfolio related info about a single Security. */
      struct SecurityEntry {

        /** The current value of the Security. */
        SecurityValuation m_valuation;

        /** The unrealized profit and loss. */
        Money m_unrealized;

        /**
         * Constructs a SecurityEntry.
         * @param currency The currency used to value the Security.
         */
        explicit SecurityEntry(CurrencyId currency) noexcept;
      };

      /** The type used to map Securities to SecurityEntries. */
      using SecurityEntryMap = std::unordered_map<Security, SecurityEntry>;

      /** The type used to map currencies to unrealized profit and losses. */
      using UnrealizedProfitAndLossMap = std::unordered_map<CurrencyId, Money>;

      /** Constructs an empty Portfolio using the default venues. */
      Portfolio();

      /**
       * Constructs an empty Portfolio.
       * @param venues The venues used to identify the currency used for an
       *        position.
       */
      explicit Portfolio(VenueDatabase venues);

      /**
       * Constructs a Portfolio for an existing set of positions using the
       * default venues.
       * @param bookkeeper The bookkeeper with the existing set of positions.
       */
      explicit Portfolio(Bookkeeper bookkeeper);

      /**
       * Constructs a Portfolio for an existing set of positions.
       * @param bookkeeper The bookkeeper with the existing set of positions.
       * @param venues The venues used to identify the currency used for an
       *        position.
       */
      Portfolio(Bookkeeper bookkeeper, VenueDatabase venues);

      /** Returns the Bookkeeper. */
      const Bookkeeper& get_bookkeeper() const;

      /** Returns the Portfolio's SecurityEntries. */
      const SecurityEntryMap& get_security_entries() const;

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
       * Updates the ask value of a Security.
       * @param security The Security whose value is being updated.
       * @param value The <i>security</i>'s ask side value.
       * @return <code>true</code> iff the update resulted in a change to
       *         the portfolio.
       */
      bool update_ask(const Security& security, Money value);

      /**
       * Updates the bid value of a Security.
       * @param security The Security whose value is being updated.
       * @param value The <i>security</i>'s bid side value.
       * @return <code>true</code> iff the update resulted in a change to
       *         the portfolio.
       */
      bool update_bid(const Security& security, Money value);

      /**
       * Updates the market value of a Security.
       * @param security The Security whose value is being updated.
       * @param ask_value The <i>security</i>'s ask side value.
       * @param bid_value The <i>security</i>'s bid side value.
       * @return <code>true</code> iff the update resulted in a change to
       *         the portfolio.
       */
      bool update(const Security& security, Money ask_value, Money bid_value);

    private:
      Bookkeeper m_bookkeeper;
      VenueDatabase m_venues;
      SecurityEntryMap m_security_entries;
      UnrealizedProfitAndLossMap m_unrealized_currencies;

      static boost::optional<Money> calculate_unrealized(
        const Inventory& inventory, const SecurityEntry& entry);
      SecurityEntry& get_entry(const Security& security);
      bool update(const Security& security, SecurityEntry& entry);
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
      const Inventory& inventory, const SecurityValuation& valuation) {
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
      const Inventory& inventory, const SecurityValuation& valuation) {
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
      const Portfolio<B>& portfolio, CurrencyId currency) {
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
    auto& security_entries = portfolio.get_security_entries();
    for(auto& security_entry_pair : security_entries) {
      auto& security = security_entry_pair.first;
      auto& security_entry = security_entry_pair.second;
      auto update = PortfolioUpdateEntry();
      update.m_security_inventory = portfolio.get_bookkeeper().get_inventory(
        security, security_entry.m_valuation.m_currency);
      if(is_empty(update.m_security_inventory)) {
        continue;
      }
      update.m_unrealized_security = security_entry.m_unrealized;
      update.m_currency_inventory = portfolio.get_bookkeeper().get_total(
        security_entry.m_valuation.m_currency);
      auto unrealized_currency_iterator =
        portfolio.get_unrealized_profit_and_losses().find(
          security_entry.m_valuation.m_currency);
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
    return out << '(' << entry.m_security_inventory << ' ' <<
      entry.m_unrealized_security << ' ' << entry.m_currency_inventory << ' ' <<
      entry.m_unrealized_currency << ')';
  }

  inline std::ostream& operator <<(
      std::ostream& out, const SecurityValuation& valuation) {
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

  inline SecurityValuation::SecurityValuation(CurrencyId currency) noexcept
    : m_currency(currency) {}

  template<IsBookkeeper B>
  Portfolio<B>::SecurityEntry::SecurityEntry(CurrencyId currency) noexcept
    : m_valuation(currency) {}

  template<IsBookkeeper B>
  Portfolio<B>::Portfolio()
    : Portfolio(DEFAULT_VENUES) {}

  template<IsBookkeeper B>
  Portfolio<B>::Portfolio(VenueDatabase venues)
    : m_venues(std::move(venues)) {}

  template<IsBookkeeper B>
  Portfolio<B>::Portfolio(Bookkeeper bookkeeper)
    : Portfolio(std::move(bookkeeper), DEFAULT_VENUES) {}

  template<IsBookkeeper B>
  Portfolio<B>::Portfolio(Bookkeeper bookkeeper, VenueDatabase venues)
      : m_bookkeeper(std::move(bookkeeper)),
        m_venues(std::move(venues)) {
    for(auto& inventory : m_bookkeeper.get_inventory_range()) {
      get_entry(inventory.m_position.m_security);
    }
  }

  template<IsBookkeeper B>
  const typename Portfolio<B>::Bookkeeper&
      Portfolio<B>::get_bookkeeper() const {
    return m_bookkeeper;
  }

  template<IsBookkeeper B>
  const typename Portfolio<B>::SecurityEntryMap&
      Portfolio<B>::get_security_entries() const {
    return m_security_entries;
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
    auto& security = fields.m_security;
    auto currency = fields.m_currency;
    auto& entry = get_entry(security);
    auto quantity = get_direction(fields.m_side) * report.m_last_quantity;
    m_bookkeeper.record(security, currency, quantity,
      report.m_last_quantity * report.m_last_price, get_fee_total(report));
    auto inventory = m_bookkeeper.get_inventory(security, currency);
    if(auto unrealized_security = calculate_unrealized(inventory, entry)) {
      auto& unrealized_currency = m_unrealized_currencies[currency];
      if(*unrealized_security != entry.m_unrealized) {
        unrealized_currency -= entry.m_unrealized;
        entry.m_unrealized = *unrealized_security;
        unrealized_currency += entry.m_unrealized;
      }
    }
    return true;
  }

  template<IsBookkeeper B>
  bool Portfolio<B>::update_ask(const Security& security, Money value) {
    auto& entry = get_entry(security);
    entry.m_valuation.m_ask_value = value;
    return update(security, entry);
  }

  template<IsBookkeeper B>
  bool Portfolio<B>::update_bid(const Security& security, Money value) {
    auto& entry = get_entry(security);
    entry.m_valuation.m_bid_value = value;
    return update(security, entry);
  }

  template<IsBookkeeper B>
  bool Portfolio<B>::update(
      const Security& security, Money ask_value, Money bid_value) {
    auto& entry = get_entry(security);
    entry.m_valuation.m_ask_value = ask_value;
    entry.m_valuation.m_bid_value = bid_value;
    return update(security, entry);
  }

  template<IsBookkeeper B>
  boost::optional<Money> Portfolio<B>::calculate_unrealized(
      const Inventory& inventory, const SecurityEntry& entry) {
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
  typename Portfolio<B>::SecurityEntry& Portfolio<B>::get_entry(
      const Security& security) {
    auto security_iterator = m_security_entries.find(security);
    if(security_iterator == m_security_entries.end()) {
      auto currency = m_venues.from(security.get_venue()).m_currency;
      security_iterator = m_security_entries.insert(
        std::pair(security, SecurityEntry(currency))).first;
    }
    return security_iterator->second;
  }

  template<IsBookkeeper B>
  bool Portfolio<B>::update(const Security& security, SecurityEntry& entry) {
    auto inventory =
      m_bookkeeper.get_inventory(security, entry.m_valuation.m_currency);
    auto unrealized_security = calculate_unrealized(inventory, entry);
    if(!unrealized_security || *unrealized_security == entry.m_unrealized) {
      return false;
    }
    auto& unrealized_currency =
      m_unrealized_currencies[entry.m_valuation.m_currency];
    unrealized_currency -= entry.m_unrealized;
    entry.m_unrealized = *unrealized_security;
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
      shuttle.shuttle("security_inventory", value.m_security_inventory);
      shuttle.shuttle("unrealized_security", value.m_unrealized_security);
      shuttle.shuttle("currency_inventory", value.m_currency_inventory);
      shuttle.shuttle("unrealized_currency", value.m_unrealized_currency);
    }
  };

  template<>
  struct Shuttle<Nexus::SecurityValuation> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::SecurityValuation& value,
        unsigned int version) const {
      shuttle.shuttle("currency", value.m_currency);
      shuttle.shuttle("ask_value", value.m_ask_value);
      shuttle.shuttle("bid_value", value.m_bid_value);
    }
  };
}

#endif
