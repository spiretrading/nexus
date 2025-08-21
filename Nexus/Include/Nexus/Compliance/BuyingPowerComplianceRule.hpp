#ifndef NEXUS_BUYING_POWER_COMPLIANCE_RULE_HPP
#define NEXUS_BUYING_POWER_COMPLIANCE_RULE_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/MultiQueueWriter.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Accounting/BuyingPowerModel.hpp"
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus::Compliance {

  /**
   * Checks that an Order submission doesn't exceed the maximum allocated buying
   * power.
   * @param <C> The type of MarketDataClient used to price Orders for buying
   *            power checks.
   */
  template<MarketDataService::IsMarketDataClient C>
  class BuyingPowerComplianceRule : public ComplianceRule {
    public:

      /**
       * The type of MarketDataClient used to price Orders for buying power
       * checks.
       */
      using MarketDataClient = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a BuyingPowerComplianceRule.
       * @param parameters The list of buying power parameters.
       * @param exchange_rates Used to convert currencies.
       * @param market_data_client Initializes the MarketDataClient used to
       *        price Orders.
       */
      template<Beam::Initializes<C> CF>
      BuyingPowerComplianceRule(
        const std::vector<ComplianceParameter>& parameters,
        const ExchangeRateTable& exchange_rates, CF&& market_data_client);

      void submit(const std::shared_ptr<
        const OrderExecutionService::Order>& order) override;

      void add(const std::shared_ptr<
        const OrderExecutionService::Order>& order) override;

    private:
      CurrencyId m_currency;
      Money m_buying_power;
      ExchangeRateTable m_exchange_rates;
      Beam::GetOptionalLocalPtr<C> m_market_data_client;
      Beam::Threading::Sync<Accounting::BuyingPowerModel> m_buying_power_model;
      Beam::MultiQueueWriter<OrderExecutionService::ExecutionReport>
        m_execution_report_queue;
      std::unordered_map<OrderExecutionService::OrderId, CurrencyId>
        m_currencies;
      Beam::SynchronizedUnorderedMap<Security,
        std::shared_ptr<Beam::StateQueue<BboQuote>>> m_bbo_quotes;

      BboQuote load_bbo_quote(const Security& security);
      Money get_expected_price(
        const OrderExecutionService::OrderFields& fields);
  };

  template<typename MarketDataClient>
  BuyingPowerComplianceRule(const std::vector<ComplianceParameter>&,
    const ExchangeRateTable&, MarketDataClient&&) ->
      BuyingPowerComplianceRule<std::remove_reference_t<MarketDataClient>>;

  /**
   * Returns a ComplianceRuleSchema representing a BuyingPowerComplianceRule.
   */
  inline ComplianceRuleSchema make_buying_power_compliance_rule_schema() {
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("currency", DefaultCurrencies::USD);
    parameters.emplace_back("buying_power", Money::ZERO);
    return ComplianceRuleSchema("buying_power", std::move(parameters));
  }

  template<MarketDataService::IsMarketDataClient C>
  template<Beam::Initializes<C> CF>
  BuyingPowerComplianceRule<C>::BuyingPowerComplianceRule(
      const std::vector<ComplianceParameter>& parameters,
      const ExchangeRateTable& exchange_rates, CF&& market_data_client)
      : m_market_data_client(std::forward<CF>(market_data_client)) {
    for(auto& parameter : parameters) {
      if(parameter.m_name == "currency") {
        m_currency = boost::get<CurrencyId>(parameter.m_value);
      } else if(parameter.m_name == "buying_power") {
        m_buying_power = boost::get<Money>(parameter.m_value);
      }
    }
  }

  template<MarketDataService::IsMarketDataClient C>
  void BuyingPowerComplianceRule<C>::submit(
      const std::shared_ptr<const OrderExecutionService::Order>& order) {
    auto& fields = order->get_info().m_fields;
    auto price = get_expected_price(fields);
    Beam::Threading::With(m_buying_power_model, [&] (auto& buying_power_model) {
      while(auto report = m_execution_report_queue.TryPop()) {
        if(report->m_last_quantity != 0) {
          auto currency = Beam::Lookup(m_currencies, report->m_id);
          if(!currency) {
            BOOST_THROW_EXCEPTION(
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
        BOOST_THROW_EXCEPTION(
          ComplianceCheckException("Currency not recognized."));
      }
      m_currencies.insert(std::pair(order->get_info().m_id, fields.m_currency));
      auto updated_buying_power = buying_power_model.submit(
        order->get_info().m_id, converted_fields, converted_price);
      if(updated_buying_power > m_buying_power) {
        auto report = OrderExecutionService::ExecutionReport();
        report.m_id = order->get_info().m_id;
        report.m_status = OrderStatus::REJECTED;
        buying_power_model.update(report);
        BOOST_THROW_EXCEPTION(
          ComplianceCheckException("Order exceeds available buying power."));
      } else {
        order->get_publisher().Monitor(m_execution_report_queue.GetWriter());
      }
    });
  }

  template<MarketDataService::IsMarketDataClient C>
  void BuyingPowerComplianceRule<C>::add(
      const std::shared_ptr<const OrderExecutionService::Order>& order) {
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
    Beam::Threading::With(m_buying_power_model, [&] (auto& buying_power_model) {
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
      order->get_publisher().Monitor(m_execution_report_queue.GetWriter());
    });
  }

  template<MarketDataService::IsMarketDataClient C>
  BboQuote BuyingPowerComplianceRule<C>::load_bbo_quote(const Security& security) {
    auto publisher = m_bbo_quotes.GetOrInsert(security, [&] {
      auto publisher = std::make_shared<Beam::StateQueue<BboQuote>>();
      MarketDataService::query_real_time_with_snapshot(
        *m_market_data_client, security, publisher);
      return publisher;
    });
    try {
      return publisher->Peek();
    } catch(const Beam::PipeBrokenException&) {
      m_bbo_quotes.Erase(security);
      BOOST_THROW_EXCEPTION(
        ComplianceCheckException("No BBO quote available."));
    }
  }

  template<MarketDataService::IsMarketDataClient C>
  Money BuyingPowerComplianceRule<C>::get_expected_price(
      const OrderExecutionService::OrderFields& fields) {
    auto bbo = load_bbo_quote(fields.m_security);
    if(fields.m_type == OrderType::LIMIT) {
      if(fields.m_price <= Money::ZERO) {
        BOOST_THROW_EXCEPTION(ComplianceCheckException("Invalid price."));
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
