#ifndef NEXUS_BUYINGPOWERCHECK_HPP
#define NEXUS_BUYINGPOWERCHECK_HPP
#include <vector>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Queues/MultiQueueReader.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Threading/Sync.hpp>
#include "Nexus/Accounting/BuyingPowerTracker.hpp"
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheck.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheckException.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class BuyingPowerCheck
      \brief Performs a buying power check on an Order submission.
      \tparam AdministrationClientType The type of AdministrationClient used to
              get the RiskParameters.
      \tparam MarketDataClientType The type of MarketDataClient used to price
              Orders for buying power checks.
   */
  template<typename AdministrationClientType, typename MarketDataClientType>
  class BuyingPowerCheck : public OrderSubmissionCheck {
    public:

      //! The type of AdministrationClient used to get the RiskParameters.
      using AdministrationClient =
        Beam::GetTryDereferenceType<AdministrationClientType>;

      //! The type of MarketDataClient used to price Orders for buying power
      //! checks.
      using MarketDataClient =
        Beam::GetTryDereferenceType<MarketDataClientType>;

      //! Constructs a BuyingPowerCheck.
      /*!
        \param exchangeRates The list of ExchangeRates.
        \param administrationClient Initializes the AdministrationClient.
        \param marketDataClient Initializes the MarketDataClient.
      */
      template<typename AdministrationClientForward,
        typename MarketDataClientForward>
      BuyingPowerCheck(const std::vector<ExchangeRate>& exchangeRates,
        AdministrationClientForward&& administrationClient,
        MarketDataClientForward&& marketDataClient);

      virtual ~BuyingPowerCheck() = default;

      virtual void Submit(const OrderInfo& orderInfo);

      virtual void Add(const Order& order);

      virtual void Reject(const OrderInfo& orderInfo);

    private:
      struct BuyingPowerEntry {
        Beam::Threading::Sync<Accounting::BuyingPowerTracker>
          m_buyingPowerTracker;
        std::shared_ptr<Beam::StateQueue<RiskService::RiskParameters>>
          m_riskParametersQueue;
        Beam::MultiQueueReader<ExecutionReport> m_executionReportQueue;
        Beam::SynchronizedUnorderedMap<OrderId, CurrencyId> m_currencies;

        BuyingPowerEntry();
      };
      ExchangeRateTable m_exchangeRates;
      Beam::GetOptionalLocalPtr<AdministrationClientType>
        m_administrationClient;
      Beam::GetOptionalLocalPtr<MarketDataClientType> m_marketDataClient;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<BuyingPowerEntry>> m_buyingPowerEntries;
      Beam::SynchronizedUnorderedMap<Security, std::shared_ptr<
        Beam::StateQueue<BboQuote>>> m_bboQuotes;

      BboQuote LoadBboQuote(const Security& security);
      Money GetExpectedPrice(const OrderFields& orderFields);
      BuyingPowerEntry& LoadBuyingPowerEntry(
        const Beam::ServiceLocator::DirectoryEntry& account);
  };

  template<typename AdministrationClientType, typename MarketDataClientType>
  BuyingPowerCheck<AdministrationClientType, MarketDataClientType>::
      BuyingPowerEntry::BuyingPowerEntry()
      : m_riskParametersQueue(std::make_shared<Beam::StateQueue<
          RiskService::RiskParameters>>()) {}

  template<typename AdministrationClientType, typename MarketDataClientType>
  template<typename AdministrationClientForward,
    typename MarketDataClientForward>
  BuyingPowerCheck<AdministrationClientType, MarketDataClientType>::
      BuyingPowerCheck(const std::vector<ExchangeRate>& exchangeRates,
      AdministrationClientForward&& administrationClient,
      MarketDataClientForward&& marketDataClient)
      : m_administrationClient{std::forward<AdministrationClientForward>(
          administrationClient)},
        m_marketDataClient{std::forward<MarketDataClientForward>(
          marketDataClient)} {
    for(auto& exchangeRate : exchangeRates) {
      m_exchangeRates.Add(exchangeRate);
    }
  }

  template<typename AdministrationClientType, typename MarketDataClientType>
  void BuyingPowerCheck<AdministrationClientType, MarketDataClientType>::Submit(
      const OrderInfo& orderInfo) {
    auto& fields = orderInfo.m_fields;
    auto price = GetExpectedPrice(fields);
    auto& buyingPowerEntry = LoadBuyingPowerEntry(fields.m_account);
    Beam::Threading::With(buyingPowerEntry.m_buyingPowerTracker,
      [&] (auto& buyingPowerTracker) {
        auto riskParameters = buyingPowerEntry.m_riskParametersQueue->Top();
        while(auto report = buyingPowerEntry.m_executionReportQueue.TryPop()) {
          if(report->m_lastQuantity != 0) {
            auto currency = buyingPowerEntry.m_currencies.Find(report->m_id);
            if(!currency) {
              BOOST_THROW_EXCEPTION(OrderSubmissionCheckException{
                "Currency not recognized."});
            }
            report->m_lastPrice = m_exchangeRates.Convert(report->m_lastPrice,
              *currency, riskParameters.m_currency);
          }
          buyingPowerTracker.Update(*report);
        }
        auto convertedFields = fields;
        convertedFields.m_currency = riskParameters.m_currency;
        Money convertedPrice;
        try {
          convertedFields.m_price = m_exchangeRates.Convert(fields.m_price,
            fields.m_currency, riskParameters.m_currency);
          convertedPrice = m_exchangeRates.Convert(price, fields.m_currency,
            riskParameters.m_currency);
        } catch(const CurrencyPairNotFoundException&) {
          BOOST_THROW_EXCEPTION(OrderSubmissionCheckException{
            "Currency not recognized."});
        }
        buyingPowerEntry.m_currencies.Insert(orderInfo.m_orderId,
          fields.m_currency);
        auto updatedBuyingPower = buyingPowerTracker.Submit(
          orderInfo.m_orderId, convertedFields, convertedPrice);
        if(updatedBuyingPower > riskParameters.m_buyingPower) {
          ExecutionReport report;
          report.m_id = orderInfo.m_orderId;
          report.m_status = OrderStatus::REJECTED;
          buyingPowerTracker.Update(report);
          BOOST_THROW_EXCEPTION(OrderSubmissionCheckException{
            "Order exceeds available buying power."});
        }
      });
  }

  template<typename AdministrationClientType, typename MarketDataClientType>
  void BuyingPowerCheck<AdministrationClientType, MarketDataClientType>::Add(
      const Order& order) {
    auto& buyingPowerEntry = LoadBuyingPowerEntry(
      order.GetInfo().m_fields.m_account);
    auto price = GetExpectedPrice(order.GetInfo().m_fields);
    Beam::Threading::With(buyingPowerEntry.m_buyingPowerTracker,
      [&] (auto& buyingPowerTracker) {
        if(buyingPowerTracker.HasOrder(order.GetInfo().m_orderId)) {
          return;
        }
        buyingPowerEntry.m_currencies.Insert(order.GetInfo().m_orderId,
          order.GetInfo().m_fields.m_currency);
        auto convertedFields = order.GetInfo().m_fields;
        convertedFields.m_currency =
          buyingPowerEntry.m_riskParametersQueue->Top().m_currency;
        Money convertedPrice;
        try {
          convertedFields.m_price = m_exchangeRates.Convert(
            order.GetInfo().m_fields.m_price,
            order.GetInfo().m_fields.m_currency, convertedFields.m_currency);
          convertedPrice = m_exchangeRates.Convert(price,
            order.GetInfo().m_fields.m_currency, convertedFields.m_currency);
        } catch(const CurrencyPairNotFoundException&) {
          BOOST_THROW_EXCEPTION(OrderSubmissionCheckException{
            "Currency not recognized."});
        }
        buyingPowerTracker.Submit(order.GetInfo().m_orderId, convertedFields,
          convertedPrice);
      });
    order.GetPublisher().Monitor(
      buyingPowerEntry.m_executionReportQueue.GetWriter());
  }

  template<typename AdministrationClientType, typename MarketDataClientType>
  void BuyingPowerCheck<AdministrationClientType, MarketDataClientType>::Reject(
      const OrderInfo& orderInfo) {
    auto& buyingPowerEntry = LoadBuyingPowerEntry(orderInfo.m_fields.m_account);
    Beam::Threading::With(buyingPowerEntry.m_buyingPowerTracker,
      [&] (auto& buyingPowerTracker) {
        if(!buyingPowerTracker.HasOrder(orderInfo.m_orderId)) {
          return;
        }
        ExecutionReport report;
        report.m_id = orderInfo.m_orderId;
        report.m_status = OrderStatus::REJECTED;
        buyingPowerTracker.Update(report);
      });
  }

  template<typename AdministrationClientType, typename MarketDataClientType>
  BboQuote BuyingPowerCheck<AdministrationClientType, MarketDataClientType>::
      LoadBboQuote(const Security& security) {
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
      BOOST_THROW_EXCEPTION(OrderSubmissionCheckException{
        "No BBO quote available."});
    }
  }

  template<typename AdministrationClientType, typename MarketDataClientType>
  Money BuyingPowerCheck<AdministrationClientType, MarketDataClientType>::
      GetExpectedPrice(const OrderFields& orderFields) {
    auto bbo = LoadBboQuote(orderFields.m_security);
    if(orderFields.m_type == OrderType::LIMIT) {
      if(orderFields.m_price <= Money::ZERO) {
        BOOST_THROW_EXCEPTION(OrderSubmissionCheckException{"Invalid price."});
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

  template<typename AdministrationClientType, typename MarketDataClientType>
  typename BuyingPowerCheck<AdministrationClientType, MarketDataClientType>::
      BuyingPowerEntry& BuyingPowerCheck<AdministrationClientType,
      MarketDataClientType>::LoadBuyingPowerEntry(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto& entry = *m_buyingPowerEntries.GetOrInsert(account,
      [&] {
        auto entry = std::make_shared<BuyingPowerEntry>();
        m_administrationClient->GetRiskParametersPublisher(account).Monitor(
          entry->m_riskParametersQueue);
        return entry;
      });
    entry.m_riskParametersQueue->Top();
    return entry;
  }
}
}

#endif
