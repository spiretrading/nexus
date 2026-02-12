#ifndef NEXUS_BUYING_POWER_COMPLIANCE_RULE_HPP
#define NEXUS_BUYING_POWER_COMPLIANCE_RULE_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/MultiQueueWriter.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Accounting/BuyingPowerModel.hpp"
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus {

  /**
   * Checks that an Order submission doesn't exceed the maximum allocated buying
   * power.
   * @param <C> The type of MarketDataClient used to price Orders for buying
   *            power checks.
   */
  template<typename C> requires
    IsMarketDataClient<Beam::dereference_t<C>>
  class BuyingPowerComplianceRule : public ComplianceRule {
    public:

      /**
       * The type of MarketDataClient used to price Orders for buying power
       * checks.
       */
      using MarketDataClient = Beam::dereference_t<C>;

      /**
       * Constructs a BuyingPowerComplianceRule.
       * @param buying_power The total buying power available.
       * @param currency The buying power's currency.
       * @param exchange_rates Used to convert currencies.
       * @param market_data_client Initializes the MarketDataClient used to
       *        price Orders.
       */
      template<Beam::Initializes<C> CF>
      BuyingPowerComplianceRule(Money buying_power, Asset currency,
        const ExchangeRateTable& exchange_rates, CF&& market_data_client);

      void submit(const std::shared_ptr<Order>& order) override;
      void add(const std::shared_ptr<Order>& order) override;

    private:
      Money m_buying_power;
      Asset m_currency;
      ExchangeRateTable m_exchange_rates;
      Beam::local_ptr_t<C> m_market_data_client;
      Beam::Sync<BuyingPowerModel> m_buying_power_model;
      Beam::MultiQueueWriter<ExecutionReport> m_execution_report_queue;
      std::unordered_map<OrderId, Asset> m_currencies;
      Beam::SynchronizedUnorderedMap<Ticker,
        std::shared_ptr<Beam::StateQueue<BboQuote>>> m_bbo_quotes;
      std::vector<Beam::RoutineHandler> m_query_routines;

      BboQuote load_bbo_quote(const Ticker& ticker);
      Money get_expected_price(const OrderFields& fields);
  };

  template<typename C>
  BuyingPowerComplianceRule(Money, Asset, const ExchangeRateTable&, C&&) ->
    BuyingPowerComplianceRule<std::remove_cvref_t<C>>;

  /** The standard name used to identify the BuyingPowerComplianceRule. */
  inline auto BUYING_POWER_COMPLIANCE_RULE_NAME = std::string("buying_power");

  /**
   * Returns a ComplianceRuleSchema representing a BuyingPowerComplianceRule.
   */
  inline ComplianceRuleSchema make_buying_power_compliance_rule_schema() {
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("buying_power", Money::ZERO);
    parameters.emplace_back("currency", DefaultCurrencies::USD);
    return ComplianceRuleSchema(
      BUYING_POWER_COMPLIANCE_RULE_NAME, std::move(parameters));
  }

  /**
   * Makes a new BuyingPowerComplianceRule from a list of ComplianceParameters.
   * @param parameters The parameters to construct the rule from.
   * @param exchange_rates Used to convert currencies.
   * @param market_data_client Initializes the MarketDataClient used to price
   *        Orders.
   */
  inline auto make_buying_power_compliance_rule(
      const std::vector<ComplianceParameter>& parameters,
      const ExchangeRateTable& exchange_rates,
      IsMarketDataClient auto& market_data_client) {
    auto buying_power = Money::ZERO;
    auto currency = Asset(DefaultCurrencies::USD);
    for(auto& parameter : parameters) {
      if(parameter.m_name == "buying_power") {
        buying_power = boost::get<Money>(parameter.m_value);
      } else if(parameter.m_name == "currency") {
        currency = boost::get<Asset>(parameter.m_value);
      }
    }
    using Rule = BuyingPowerComplianceRule<
      std::remove_reference_t<decltype(market_data_client)>*>;
    return std::make_unique<Rule>(
      buying_power, currency, exchange_rates, &market_data_client);
  }

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  template<Beam::Initializes<C> CF>
  BuyingPowerComplianceRule<C>::BuyingPowerComplianceRule(Money buying_power,
    Asset currency, const ExchangeRateTable& exchange_rates,
    CF&& market_data_client)
    : m_buying_power(buying_power),
      m_currency(currency),
      m_market_data_client(std::forward<CF>(market_data_client)) {}

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  void BuyingPowerComplianceRule<C>::submit(
      const std::shared_ptr<Order>& order) {
    auto& fields = order->get_info().m_fields;
    auto price = get_expected_price(fields);
    Beam::with(m_buying_power_model, [&] (auto& buying_power_model) {
      while(auto report = m_execution_report_queue.try_pop()) {
        if(report->m_last_quantity != 0) {
          auto currency = Beam::lookup(m_currencies, report->m_id);
          if(!currency) {
            boost::throw_with_location(
              ComplianceCheckException("Currency not recognized."));
          }
          report->m_last_price = m_exchange_rates.convert(
            report->m_last_price, *currency, m_currency);
        }
        buying_power_model.update(*report);
      }
      auto converted_fields = fields;
      converted_fields.m_currency = m_currency;
      auto converted_price = Money();
      try {
        converted_fields.m_price = m_exchange_rates.convert(
          fields.m_price, fields.m_currency, m_currency);
        converted_price =
          m_exchange_rates.convert(price, fields.m_currency, m_currency);
      } catch(const CurrencyPairNotFoundException&) {
        boost::throw_with_location(
          ComplianceCheckException("Currency not recognized."));
      }
      m_currencies.insert(std::pair(order->get_info().m_id, fields.m_currency));
      auto updated_buying_power = buying_power_model.submit(
        order->get_info().m_id, converted_fields, converted_price);
      if(updated_buying_power > m_buying_power) {
        auto report = ExecutionReport();
        report.m_id = order->get_info().m_id;
        report.m_status = OrderStatus::REJECTED;
        buying_power_model.update(report);
        boost::throw_with_location(
          ComplianceCheckException("Order exceeds available buying power."));
      } else {
        order->get_publisher().monitor(m_execution_report_queue.get_writer());
      }
    });
  }

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  void BuyingPowerComplianceRule<C>::add(const std::shared_ptr<Order>& order) {
    auto& fields = order->get_info().m_fields;
    auto price = [&] {
      try {
        return get_expected_price(order->get_info().m_fields);
      } catch(const std::exception&) {
        if(order->get_info().m_fields.m_type == OrderType::LIMIT) {
          return order->get_info().m_fields.m_price;
        }
        return Money::ZERO;
      }
    }();
    Beam::with(m_buying_power_model, [&] (auto& buying_power_model) {
      auto converted_fields = fields;
      m_currencies.insert(std::pair(order->get_info().m_id, fields.m_currency));
      converted_fields.m_currency = m_currency;
      auto converted_price = Money();
      try {
        converted_fields.m_price = m_exchange_rates.convert(
          fields.m_price, fields.m_currency, converted_fields.m_currency);
        converted_price = m_exchange_rates.convert(
          price, fields.m_currency, converted_fields.m_currency);
      } catch(const CurrencyPairNotFoundException&) {
        return;
      }
      buying_power_model.submit(
        order->get_info().m_id, converted_fields, converted_price);
      order->get_publisher().monitor(m_execution_report_queue.get_writer());
    });
  }

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  BboQuote BuyingPowerComplianceRule<C>::load_bbo_quote(const Ticker& ticker) {
    auto publisher = m_bbo_quotes.get_or_insert(ticker, [&] {
      auto publisher = std::make_shared<Beam::StateQueue<BboQuote>>();
      m_query_routines.push_back(query_real_time_with_snapshot(
        *m_market_data_client, ticker, publisher));
      return publisher;
    });
    try {
      return publisher->peek();
    } catch(const Beam::PipeBrokenException&) {
      m_bbo_quotes.erase(ticker);
      boost::throw_with_location(
        ComplianceCheckException("No BBO quote available."));
    }
  }

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  Money BuyingPowerComplianceRule<C>::get_expected_price(
      const OrderFields& fields) {
    auto bbo = load_bbo_quote(fields.m_ticker);
    if(fields.m_type == OrderType::LIMIT) {
      if(fields.m_price <= Money::ZERO) {
        boost::throw_with_location(ComplianceCheckException("Invalid price."));
      }
      if(fields.m_side == Side::ASK) {
        return std::max(bbo.m_bid.m_price, fields.m_price);
      } else {
        return std::min(bbo.m_ask.m_price, fields.m_price);
      }
    } else {
      if(fields.m_side == Side::ASK) {
        return bbo.m_bid.m_price;
      } else {
        return bbo.m_ask.m_price;
      }
    }
  }
}

#endif
