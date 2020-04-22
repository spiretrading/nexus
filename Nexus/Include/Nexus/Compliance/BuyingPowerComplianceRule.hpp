#ifndef NEXUS_BUYINGPOWERCOMPLIANCERULE_HPP
#define NEXUS_BUYINGPOWERCOMPLIANCERULE_HPP
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/MultiQueueReader.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Accounting/BuyingPowerTracker.hpp"
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class BuyingPowerComplianceRule
      \brief Checks that an Order submission doesn't exceed the maximum
             allocated buying power.
      \tparam MarketDataClientType The type of MarketDataClient used to price
              Orders for buying power checks.
   */
  template<typename MarketDataClientType>
  class BuyingPowerComplianceRule : public ComplianceRule {
    public:

      //! The type of MarketDataClient used to price Orders for buying power
      //! checks.
      using MarketDataClient =
        Beam::GetTryDereferenceType<MarketDataClientType>;

      //! Constructs a BuyingPowerComplianceRule.
      /*!
        \param parameters The list of buying power parameters.
        \param exchangeRates The list of ExchangeRates.
        \param marketDataClient Initializes the MarketDataClient used to price
               Orders.
      */
      template<typename MarketDataClientForward>
      BuyingPowerComplianceRule(
        const std::vector<ComplianceParameter>& parameters,
        const std::vector<ExchangeRate>& exchangeRates,
        MarketDataClientForward&& marketDataClient);

      virtual void Submit(const OrderExecutionService::Order& order);

      virtual void Add(const OrderExecutionService::Order& order);

    private:
      CurrencyId m_currency;
      Money m_buyingPower;
      SecuritySet m_securities;
      ExchangeRateTable m_exchangeRates;
      Beam::GetOptionalLocalPtr<MarketDataClientType> m_marketDataClient;
      Beam::Threading::Sync<Accounting::BuyingPowerTracker>
        m_buyingPowerTracker;
      Beam::MultiQueueReader<OrderExecutionService::ExecutionReport>
        m_executionReportQueue;
      std::unordered_map<OrderExecutionService::OrderId, CurrencyId>
        m_currencies;
      Beam::SynchronizedUnorderedMap<Security,
        std::shared_ptr<Beam::StateQueue<BboQuote>>> m_bboQuotes;

      BboQuote LoadBboQuote(const Security& security);
      Money GetExpectedPrice(
        const OrderExecutionService::OrderFields& orderFields);
  };


  template<typename MarketDataClient>
  BuyingPowerComplianceRule(const std::vector<ComplianceParameter>&,
    const std::vector<ExchangeRate>&, MarketDataClient&&) ->
    BuyingPowerComplianceRule<std::decay_t<MarketDataClient>>;

  //! Builds a ComplianceRuleSchema representing a BuyingPowerComplianceRule.
  inline ComplianceRuleSchema BuildBuyingPowerComplianceRuleSchema() {
    std::vector<ComplianceParameter> parameters;
    parameters.emplace_back("currency", DefaultCurrencies::USD());
    parameters.emplace_back("buying_power", Money::ZERO);
    std::vector<ComplianceValue> symbols;
    symbols.push_back(Security{"*", MarketCode{}, CountryDatabase::NONE});
    parameters.emplace_back("symbols", symbols);
    ComplianceRuleSchema schema{"buying_power", parameters};
    return schema;
  }

  template<typename MarketDataClientType>
  template<typename MarketDataClientForward>
  BuyingPowerComplianceRule<MarketDataClientType>::BuyingPowerComplianceRule(
      const std::vector<ComplianceParameter>& parameters,
      const std::vector<ExchangeRate>& exchangeRates,
      MarketDataClientForward&& marketDataClient)
      : m_marketDataClient(std::forward<MarketDataClientForward>(
          marketDataClient)) {
    for(auto& parameter : parameters) {
      if(parameter.m_name == "currency") {
        m_currency = boost::get<CurrencyId>(parameter.m_value);
      } else if(parameter.m_name == "buying_power") {
        m_buyingPower = boost::get<Money>(parameter.m_value);
      } else if(parameter.m_name == "symbols") {
        for(auto& security : boost::get<std::vector<ComplianceValue>>(
            parameter.m_value)) {
          m_securities.Add(std::move(boost::get<Security>(security)));
        }
      }
    }
    if(m_securities.IsEmpty()) {
      m_securities.Add(SecuritySet::GetSecurityWildCard());
    }
    for(auto& exchangeRate : exchangeRates) {
      m_exchangeRates.Add(exchangeRate);
    }
  }

  template<typename MarketDataClientType>
  void BuyingPowerComplianceRule<MarketDataClientType>::Submit(
      const OrderExecutionService::Order& order) {
    auto& fields = order.GetInfo().m_fields;
    if(!m_securities.Contains(fields.m_security)) {
      return;
    }
    auto price = GetExpectedPrice(fields);
    Beam::Threading::With(m_buyingPowerTracker,
      [&] (auto& buyingPowerTracker) {
        while(!m_executionReportQueue.IsEmpty()) {
          auto report = m_executionReportQueue.Top();
          m_executionReportQueue.Pop();
          if(report.m_lastQuantity != 0) {
            auto currency = Beam::Lookup(m_currencies, report.m_id);
            if(!currency.is_initialized()) {
              BOOST_THROW_EXCEPTION(ComplianceCheckException{
                "Currency not recognized."});
            }
            report.m_lastPrice = m_exchangeRates.Convert(report.m_lastPrice,
              *currency, m_currency);
          }
          buyingPowerTracker.Update(report);
        }
        auto convertedFields = fields;
        convertedFields.m_currency = m_currency;
        Money convertedPrice;
        try {
          convertedFields.m_price = m_exchangeRates.Convert(fields.m_price,
            fields.m_currency, m_currency);
          convertedPrice = m_exchangeRates.Convert(price, fields.m_currency,
            m_currency);
        } catch(const CurrencyPairNotFoundException&) {
          BOOST_THROW_EXCEPTION(ComplianceCheckException{
            "Currency not recognized."});
        }
        m_currencies.insert(std::make_pair(order.GetInfo().m_orderId,
          fields.m_currency));
        auto updatedBuyingPower = buyingPowerTracker.Submit(
          order.GetInfo().m_orderId, convertedFields, convertedPrice);
        if(updatedBuyingPower > m_buyingPower) {
          OrderExecutionService::ExecutionReport report;
          report.m_id = order.GetInfo().m_orderId;
          report.m_status = OrderStatus::REJECTED;
          buyingPowerTracker.Update(report);
          BOOST_THROW_EXCEPTION(ComplianceCheckException{
            "Order exceeds available buying power."});
        } else {
          order.GetPublisher().Monitor(m_executionReportQueue.GetWriter());
        }
    });
  }

  template<typename MarketDataClientType>
  void BuyingPowerComplianceRule<MarketDataClientType>::Add(
      const OrderExecutionService::Order& order) {
    auto& fields = order.GetInfo().m_fields;
    if(!m_securities.Contains(fields.m_security)) {
      return;
    }
    auto price = GetExpectedPrice(fields);
    Beam::Threading::With(m_buyingPowerTracker,
      [&] (auto& buyingPowerTracker) {
        auto convertedFields = fields;
        m_currencies.insert(std::make_pair(order.GetInfo().m_orderId,
          fields.m_currency));
        convertedFields.m_currency = m_currency;
        Money convertedPrice;
        try {
          convertedFields.m_price = m_exchangeRates.Convert(fields.m_price,
            fields.m_currency, convertedFields.m_currency);
          convertedPrice = m_exchangeRates.Convert(price, fields.m_currency,
            fields.m_currency);
        } catch(const CurrencyPairNotFoundException&) {
          return;
        }
        buyingPowerTracker.Submit(order.GetInfo().m_orderId, convertedFields,
          convertedPrice);
        order.GetPublisher().Monitor(m_executionReportQueue.GetWriter());
    });
  }

  template<typename MarketDataClientType>
  BboQuote BuyingPowerComplianceRule<MarketDataClientType>::LoadBboQuote(
      const Security& security) {
    auto publisher = m_bboQuotes.GetOrInsert(security,
      [&] {
        auto publisher = std::make_shared<Beam::StateQueue<BboQuote>>();
        MarketDataService::QueryRealTimeWithSnapshot(security,
          *m_marketDataClient, publisher);
        return publisher;
      });
    try {
      return publisher->Top();
    } catch(const Beam::PipeBrokenException&) {
      m_bboQuotes.Erase(security);
      BOOST_THROW_EXCEPTION(ComplianceCheckException{
        "No BBO quote available."});
    }
  }

  template<typename MarketDataClientType>
  Money BuyingPowerComplianceRule<MarketDataClientType>::GetExpectedPrice(
      const OrderExecutionService::OrderFields& orderFields) {
    auto bbo = LoadBboQuote(orderFields.m_security);
    if(orderFields.m_type == OrderType::LIMIT) {
      if(orderFields.m_price <= Money::ZERO) {
        BOOST_THROW_EXCEPTION(ComplianceCheckException{"Invalid price."});
      }
      if(orderFields.m_side == Side::ASK) {
        return std::max(bbo.m_bid.m_price, orderFields.m_price);
      } else {
        return std::min(bbo.m_ask.m_price, orderFields.m_price);
      }
    } else {
      if(orderFields.m_side == Side::ASK) {
        return bbo.m_bid.m_price;
      } else {
        return bbo.m_ask.m_price;
      }
    }
  }
}
}

#endif
