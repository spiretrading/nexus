#ifndef NEXUS_PORTFOLIO_HPP
#define NEXUS_PORTFOLIO_HPP
#include <unordered_map>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Accounting/Accounting.hpp"
#include "Nexus/Accounting/Bookkeeper.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus::Accounting {

  /** Stores an update to a Portfolio's snapshot. */
  template<typename I>
  struct PortfolioUpdateEntry {

    /** The type of Inventory used. */
    using Inventory = I;

    /** The updated Security Inventory. */
    Inventory m_securityInventory;

    /** The updated unrealized gross earnings for the Security. */
    Money m_unrealizedSecurity;

    /** The updated Currency Inventory. */
    Inventory m_currencyInventory;

    /** The updated unrealized gross earnings for the Currency. */
    Money m_unrealizedCurrency;
  };

  /** Stores the valuation used for a Security. */
  struct SecurityValuation {

    /** The currency used to value the Security. */
    CurrencyId m_currency;

    /** The ask side value. */
    boost::optional<Money> m_askValue;

    /** The bid side value. */
    boost::optional<Money> m_bidValue;

    /** Constructs a SecurityValuation. */
    SecurityValuation() = default;

    /**
     * Constructs a SecurityValuation.
     * @param currency The currency used to value the Security.
     */
    explicit SecurityValuation(CurrencyId currency);
  };

  /**
   * Monitors transactions as part of a portfolio and reports the P/L.
   * @param <B> The type of Bookkeeper to use.
   */
  template<typename B>
  class Portfolio {
    public:

      /** The type of Bookkeeper to use. */
      using PortfolioBookkeeper = B;

      /** The type of Inventory stored by the Portfolio. */
      using Inventory = typename PortfolioBookkeeper::Inventory;

      /** The type of update published. */
      using UpdateEntry = PortfolioUpdateEntry<Inventory>;

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
        explicit SecurityEntry(CurrencyId currency);
      };

      /** The type used to map Securities to SecurityEntries. */
      using SecurityEntryMap = std::unordered_map<Security, SecurityEntry>;

      /** The type used to map currencies to unrealized profit and losses. */
      using UnrealizedProfitAndLossMap = std::unordered_map<CurrencyId, Money>;

      /**
       * Constructs a Portfolio.
       * @param markets The database of markets used.
       */
      explicit Portfolio(const MarketDatabase& markets);

      /** Returns the Bookkeeper. */
      const PortfolioBookkeeper& GetBookkeeper() const;

      /** Returns the Portfolio's SecurityEntries. */
      const SecurityEntryMap& GetSecurityEntries() const;

      /** Returns the Portfolio's unrealized profit and losses. */
      const UnrealizedProfitAndLossMap& GetUnrealizedProfitAndLosses() const;

      /**
       * Updates a position held by this Portfolio.
       * @param orderFields Specifies the Order's details.
       * @param executionReport The ExecutionReport specifying the update.
       */
      void Update(const OrderExecutionService::OrderFields& orderFields,
        const OrderExecutionService::ExecutionReport& executionReport);

      /**
       * Updates the ask value of a Security.
       * @param security The Security whose value is being updated.
       * @param value The <i>security</i>'s ask side value.
       */
      void UpdateAsk(const Security& security, Money value);

      /**
       * Updates the bid value of a Security.
       * @param security The Security whose value is being updated.
       * @param value The <i>security</i>'s bid side value.
       */
      void UpdateBid(const Security& security, Money value);

      /**
       * Updates the market value of a Security.
       * @param security The Security whose value is being updated.
       * @param askValue The <i>security</i>'s ask side value.
       * @param bidValue The <i>security</i>'s bid side value.
       */
      void Update(const Security& security, Money askValue, Money bidValue);

    private:
      MarketDatabase m_marketDatabase;
      PortfolioBookkeeper m_bookkeeper;
      SecurityEntryMap m_securityEntries;
      UnrealizedProfitAndLossMap m_unrealizedCurrencies;

      static Money CalculateUnrealized(
        const typename PortfolioBookkeeper::Inventory& inventory,
        const SecurityEntry& securityEntry);
      SecurityEntry& GetSecurityEntry(const Security& security);
  };

  /**
   * Returns the realized profit and loss of a single Inventory.
   * @param inventory The Inventory to calculate the realized profit and loss
   *        of.
   * @return The <i>inventory</i>'s profit and loss.
   */
  template<typename PositionType>
  Money GetRealizedProfitAndLoss(const Inventory<PositionType>& inventory) {
    return inventory.m_grossProfitAndLoss - inventory.m_fees;
  }

  /**
   * Returns the unrealized profit and loss of a single Inventory.
   * @param inventory The Inventory to calculate the unrealized profit and loss
   *        of.
   * @param valuation The valuation to use on the <i>inventory</i>.
   * @return The <i>inventory</i>'s unrealized profit and loss.
   */
  template<typename PositionType>
  boost::optional<Money> GetUnrealizedProfitAndLoss(
      const Inventory<PositionType>& inventory,
      const SecurityValuation& valuation) {
    if(inventory.m_position.m_quantity == 0) {
      return Money::ZERO;
    }
    auto price = Money();
    if(inventory.m_position.m_quantity > 0) {
      if(valuation.m_bidValue.is_initialized()) {
        price = *valuation.m_bidValue;
      } else {
        return boost::none;
      }
    } else {
      if(valuation.m_askValue.is_initialized()) {
        price = *valuation.m_askValue;
      } else {
        return boost::none;
      }
    }
    return inventory.m_position.m_quantity * price -
      inventory.m_position.m_costBasis;
  }

  /**
   * Returns the total (realized and unrealized) profit and loss of a single
   * Inventory.
   * @param inventory The Inventory to calculate the total profit and loss of.
   * @param valuation The valuation to use on the <i>inventory</i>.
   * @return The <i>inventory</i>'s total profit and loss.
   */
  template<typename Position>
  boost::optional<Money> GetTotalProfitAndLoss(
      const Inventory<Position>& inventory,
      const SecurityValuation& valuation) {
    auto unrealizedProfitAndLoss =
      GetUnrealizedProfitAndLoss(inventory, valuation);
    if(!unrealizedProfitAndLoss.is_initialized()) {
      return boost::none;
    }
    return GetRealizedProfitAndLoss(inventory) + *unrealizedProfitAndLoss;
  }

  /**
   * Returns the total (realized and unrealized) profit and loss of all
   * Inventory in a given currency within a Portfolio.
   * @param portfolio The Portfolio to calculate the total profit and loss for.
   * @param currency The currency to calculate the total profit and loss for.
   * @return The total profit and loss of  all Inventory in the <i>portfolio</i>
   *         for the specified <i>currency</i>.
   */
  template<typename Portfolio>
  Money GetTotalProfitAndLoss(const Portfolio& portfolio, CurrencyId currency) {
    auto unrealizedProfitAndLoss = Money();
    auto profitAndLossIterator = portfolio.GetUnrealizedProfitAndLosses().find(
      currency);
    if(profitAndLossIterator ==
        portfolio.GetUnrealizedProfitAndLosses().end()) {
      unrealizedProfitAndLoss = Money::ZERO;
    } else {
      unrealizedProfitAndLoss = profitAndLossIterator->second;
    }
    auto& inventory = portfolio.GetBookkeeper().GetTotal(currency);
    return unrealizedProfitAndLoss + GetRealizedProfitAndLoss(inventory);
  }

  /**
   * Calls a function for each entry in a Portfolio.
   * @param portfolio The Portfolio to get entries from.
   * @param f The function to call with a Portfolio's entry.
   */
  template<typename Portfolio, typename F>
  void ForEachPortfolioEntry(const Portfolio& portfolio, F&& f) {
    auto& securityEntries = portfolio.GetSecurityEntries();
    for(auto& securityEntryPair : securityEntries) {
      auto& security = securityEntryPair.first;
      auto& securityEntry = securityEntryPair.second;
      auto update = typename Portfolio::UpdateEntry();
      update.m_securityInventory = portfolio.GetBookkeeper().GetInventory(
        security, securityEntry.m_valuation.m_currency);
      update.m_unrealizedSecurity = securityEntry.m_unrealized;
      update.m_currencyInventory = portfolio.GetBookkeeper().GetTotal(
        securityEntry.m_valuation.m_currency);
      auto unrealizedCurrencyIterator =
        portfolio.GetUnrealizedProfitAndLosses().find(
        securityEntry.m_valuation.m_currency);
      if(unrealizedCurrencyIterator ==
          portfolio.GetUnrealizedProfitAndLosses().end()) {
        update.m_unrealizedCurrency = Money::ZERO;
      } else {
        update.m_unrealizedCurrency = unrealizedCurrencyIterator->second;
      }
      f(std::move(update));
    }
  }

  inline SecurityValuation::SecurityValuation(CurrencyId currency)
    : m_currency(currency) {}

  template<typename B>
  Portfolio<B>::SecurityEntry::SecurityEntry(CurrencyId currency)
    : m_valuation(currency) {}

  template<typename B>
  Portfolio<B>::Portfolio(const MarketDatabase& marketDatabase)
    : m_marketDatabase(marketDatabase) {}

  template<typename B>
  const typename Portfolio<B>::PortfolioBookkeeper&
      Portfolio<B>::GetBookkeeper() const {
    return m_bookkeeper;
  }

  template<typename B>
  const typename Portfolio<B>::SecurityEntryMap&
      Portfolio<B>::GetSecurityEntries() const {
    return m_securityEntries;
  }

  template<typename B>
  const typename Portfolio<B>::UnrealizedProfitAndLossMap&
      Portfolio<B>::GetUnrealizedProfitAndLosses() const {
    return m_unrealizedCurrencies;
  }

  template<typename B>
  void Portfolio<B>::Update(
      const OrderExecutionService::OrderFields& orderFields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return;
    }
    auto& security = orderFields.m_security;
    auto currency = orderFields.m_currency;
    auto& securityEntry = GetSecurityEntry(security);
    auto quantity = GetDirection(orderFields.m_side) *
      executionReport.m_lastQuantity;
    m_bookkeeper.RecordTransaction(security, currency, quantity,
      executionReport.m_lastQuantity * executionReport.m_lastPrice,
      executionReport.m_commission + executionReport.m_executionFee +
      executionReport.m_processingFee);
    auto securityInventory = m_bookkeeper.GetInventory(security, currency);
    auto unrealizedSecurity = CalculateUnrealized(securityInventory,
      securityEntry);
    auto& unrealizedCurrency = m_unrealizedCurrencies[currency];
    if(unrealizedSecurity != securityEntry.m_unrealized) {
      unrealizedCurrency -= securityEntry.m_unrealized;
      securityEntry.m_unrealized = unrealizedSecurity;
      unrealizedCurrency += securityEntry.m_unrealized;
    }
  }

  template<typename B>
  void Portfolio<B>::UpdateAsk(const Security& security, Money value) {
    auto& securityEntry = GetSecurityEntry(security);
    securityEntry.m_valuation.m_askValue = value;
    auto securityInventory = m_bookkeeper.GetInventory(security,
      securityEntry.m_valuation.m_currency);
    auto unrealizedSecurity = CalculateUnrealized(securityInventory,
      securityEntry);
    if(unrealizedSecurity == securityEntry.m_unrealized) {
      return;
    }
    auto& unrealizedCurrency =
      m_unrealizedCurrencies[securityEntry.m_valuation.m_currency];
    unrealizedCurrency -= securityEntry.m_unrealized;
    securityEntry.m_unrealized = unrealizedSecurity;
    unrealizedCurrency += securityEntry.m_unrealized;
  }

  template<typename B>
  void Portfolio<B>::UpdateBid(const Security& security, Money value) {
    auto& securityEntry = GetSecurityEntry(security);
    securityEntry.m_valuation.m_bidValue = value;
    auto securityInventory = m_bookkeeper.GetInventory(security,
      securityEntry.m_valuation.m_currency);
    auto unrealizedSecurity = CalculateUnrealized(securityInventory,
      securityEntry);
    if(unrealizedSecurity == securityEntry.m_unrealized) {
      return;
    }
    auto& unrealizedCurrency =
      m_unrealizedCurrencies[securityEntry.m_valuation.m_currency];
    unrealizedCurrency -= securityEntry.m_unrealized;
    securityEntry.m_unrealized = unrealizedSecurity;
    unrealizedCurrency += securityEntry.m_unrealized;
  }

  template<typename B>
  void Portfolio<B>::Update(const Security& security, Money askValue,
      Money bidValue) {
    auto& securityEntry = GetSecurityEntry(security);
    securityEntry.m_valuation.m_askValue = askValue;
    securityEntry.m_valuation.m_bidValue = bidValue;
    auto securityInventory = m_bookkeeper.GetInventory(security,
      securityEntry.m_valuation.m_currency);
    auto unrealizedSecurity = CalculateUnrealized(securityInventory,
      securityEntry);
    if(unrealizedSecurity == securityEntry.m_unrealized) {
      return;
    }
    auto& unrealizedCurrency =
      m_unrealizedCurrencies[securityEntry.m_valuation.m_currency];
    unrealizedCurrency -= securityEntry.m_unrealized;
    securityEntry.m_unrealized = unrealizedSecurity;
    unrealizedCurrency += securityEntry.m_unrealized;
  }

  template<typename B>
  Money Portfolio<B>::CalculateUnrealized(
      const typename PortfolioBookkeeper::Inventory& inventory,
      const SecurityEntry& securityEntry) {
    auto valuation = [&] {
      if(inventory.m_position.m_quantity >= 0) {
        if(securityEntry.m_valuation.m_bidValue.is_initialized()) {
          return *securityEntry.m_valuation.m_bidValue;
        }
      } else {
        if(securityEntry.m_valuation.m_askValue.is_initialized()) {
          return *securityEntry.m_valuation.m_askValue;
        }
      }
      return Money::ZERO;
    }();
    return inventory.m_position.m_quantity * valuation -
      inventory.m_position.m_costBasis;
  }

  template<typename B>
  typename Portfolio<B>::SecurityEntry& Portfolio<B>::GetSecurityEntry(
      const Security& security) {
    auto securityIterator = m_securityEntries.find(security);
    if(securityIterator == m_securityEntries.end()) {
      auto currency = m_marketDatabase.FromCode(
        security.GetMarket()).m_currency;
      securityIterator = m_securityEntries.insert(
        std::pair(security, SecurityEntry(currency))).first;
    }
    return securityIterator->second;
  }
}

namespace Beam::Serialization {
  template<typename I>
  struct Shuttle<Nexus::Accounting::PortfolioUpdateEntry<I>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::Accounting::PortfolioUpdateEntry<I>& value,
        unsigned int version) {
      shuttle.Shuttle("security_inventory", value.m_securityInventory);
      shuttle.Shuttle("unrealized_security", value.m_unrealizedSecurity);
      shuttle.Shuttle("currency_inventory", value.m_currencyInventory);
      shuttle.Shuttle("unrealized_currency", value.m_unrealizedCurrency);
    }
  };

  template<>
  struct Shuttle<Nexus::Accounting::SecurityValuation> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::Accounting::SecurityValuation& value, unsigned int version) {
      shuttle.Shuttle("currency", value.m_currency);
      shuttle.Shuttle("ask_value", value.m_askValue);
      shuttle.Shuttle("bid_value", value.m_bidValue);
    }
  };
}

#endif
