#ifndef NEXUS_BUYING_POWER_CHECK_HPP
#define NEXUS_BUYING_POWER_CHECK_HPP
#include <type_traits>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/MultiQueueWriter.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Threading/Sync.hpp>
#include "Nexus/Accounting/BuyingPowerModel.hpp"
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheck.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheckException.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * Performs a buying power check on an Order submission.
   * @param <A> The type of AdministrationClient used to get the RiskParameters.
   * @param <M> The type of MarketDataClient used to price Orders for buying
   *        power checks.
   */
  template<typename A, typename M>
  class BuyingPowerCheck : public OrderSubmissionCheck {
    public:

      /** The type of AdministrationClient used to get the RiskParameters. */
      using AdministrationClient = Beam::GetTryDereferenceType<A>;

      /**
       * The type of MarketDataClient used to price Orders for buying power
       * checks.
       */
      using MarketDataClient = Beam::GetTryDereferenceType<M>;

      /**
       * Constructs a BuyingPowerCheck.
       * @param exchange_rates The list of ExchangeRates.
       * @param administration_client Initializes the AdministrationClient.
       * @param market_data_client Initializes the MarketDataClient.
       */
      template<typename AF, typename MF>
      BuyingPowerCheck(const ExchangeRateTable& exchange_rates,
        AF&& administration_client, MF&& market_data_client);

      void submit(const OrderInfo& info) override;

      void add(const std::shared_ptr<const Order>& order) override;

      void reject(const OrderInfo& info) override;

    private:
      struct BuyingPowerEntry {
        Beam::Threading::Sync<Accounting::BuyingPowerModel>
          m_buying_power_model;
        std::shared_ptr<Beam::StateQueue<RiskService::RiskParameters>>
          m_risk_parameters_queue;
        Beam::MultiQueueWriter<ExecutionReport> m_execution_report_queue;
        Beam::SynchronizedUnorderedMap<OrderId, CurrencyId> m_currencies;

        BuyingPowerEntry();
      };
      ExchangeRateTable m_exchange_rates;
      Beam::GetOptionalLocalPtr<A> m_administration_client;
      Beam::GetOptionalLocalPtr<M> m_market_data_client;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<BuyingPowerEntry>> m_buying_power_entries;
      Beam::SynchronizedUnorderedMap<
        Security, std::shared_ptr<Beam::StateQueue<BboQuote>>> m_bbo_quotes;

      BboQuote load_bbo_quote(const Security& security);
      Money get_expected_price(const OrderFields& fields);
      BuyingPowerEntry& load_buying_power_entry(
        const Beam::ServiceLocator::DirectoryEntry& account);
  };

  template<typename AdministrationClient, typename MarketDataClient>
  auto make_buying_power_check(const ExchangeRateTable& exchange_rates,
      AdministrationClient&& administration_client,
      MarketDataClient&& market_data_client) {
    return std::make_unique<BuyingPowerCheck<
      std::decay_t<AdministrationClient>, std::decay_t<MarketDataClient>>>(
        exchange_rates,
        std::forward<AdministrationClient>(administration_client),
        std::forward<MarketDataClient>(market_data_client));
  }

  /**
   * Makes a BuyingPowerCheck.
   * @param exchange_rates The list of ExchangeRates.
   * @param administration_client Initializes the AdministrationClient.
   * @param market_data_client Initializes the MarketDataClient.
   */
  template<typename A, typename M>
  BuyingPowerCheck<A, M>::BuyingPowerEntry::BuyingPowerEntry()
    : m_risk_parameters_queue(
        std::make_shared<Beam::StateQueue<RiskService::RiskParameters>>()) {}

  template<typename A, typename M>
  template<typename AF, typename MF>
  BuyingPowerCheck<A, M>::BuyingPowerCheck(
    const ExchangeRateTable& exchange_rates, AF&& administration_client,
    MF&& market_data_client)
    : m_exchange_rates(exchange_rates),
      m_administration_client(std::forward<AF>(administration_client)),
      m_market_data_client(std::forward<MF>(market_data_client)) {}

  template<typename A, typename M>
  void BuyingPowerCheck<A, M>::submit(const OrderInfo& info) {
    auto& fields = info.m_fields;
    auto price = get_expected_price(fields);
    auto& buying_power_entry = load_buying_power_entry(fields.m_account);
    Beam::Threading::With(
      buying_power_entry.m_buying_power_model, [&] (auto& buying_power_model) {
        auto risk_parameters =
          buying_power_entry.m_risk_parameters_queue->Peek();
        while(auto report =
            buying_power_entry.m_execution_report_queue.TryPop()) {
          if(report->m_last_quantity != 0) {
            auto currency =
              buying_power_entry.m_currencies.FindValue(report->m_id);
            if(!currency) {
              BOOST_THROW_EXCEPTION(
                OrderSubmissionCheckException("Currency not recognized."));
            }
            report->m_last_price = m_exchange_rates.convert(
              report->m_last_price, *currency, risk_parameters.m_currency);
          }
          buying_power_model.update(*report);
        }
        auto converted_fields = fields;
        converted_fields.m_currency = risk_parameters.m_currency;
        auto converted_price = Money();
        try {
          converted_fields.m_price = m_exchange_rates.convert(
            fields.m_price, fields.m_currency, risk_parameters.m_currency);
          converted_price = m_exchange_rates.convert(
            price, fields.m_currency, risk_parameters.m_currency);
        } catch(const CurrencyPairNotFoundException&) {
          BOOST_THROW_EXCEPTION(
            OrderSubmissionCheckException("Currency not recognized."));
        }
        buying_power_entry.m_currencies.Insert(info.m_id, fields.m_currency);
        auto updated_buying_power = buying_power_model.submit(
          info.m_id, converted_fields, converted_price);
        if(updated_buying_power > risk_parameters.m_buyingPower) {
          auto report = ExecutionReport();
          report.m_id = info.m_id;
          report.m_status = OrderStatus::REJECTED;
          buying_power_model.update(report);
          BOOST_THROW_EXCEPTION(OrderSubmissionCheckException(
            "Order exceeds available buying power."));
        }
      });
  }

  template<typename A, typename M>
  void BuyingPowerCheck<A, M>::add(const std::shared_ptr<const Order>& order) {
    auto& buying_power_entry =
      load_buying_power_entry(order->get_info().m_fields.m_account);
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
    Beam::Threading::With(
      buying_power_entry.m_buying_power_model, [&] (auto& buying_power_model) {
        if(buying_power_model.has_order(order->get_info().m_id)) {
          return;
        }
        buying_power_entry.m_currencies.Insert(
          order->get_info().m_id, order->get_info().m_fields.m_currency);
        auto converted_fields = order->get_info().m_fields;
        converted_fields.m_currency =
          buying_power_entry.m_risk_parameters_queue->Peek().m_currency;
        auto converted_price = Money();
        try {
          converted_fields.m_price = m_exchange_rates.convert(
            order->get_info().m_fields.m_price,
            order->get_info().m_fields.m_currency, converted_fields.m_currency);
          converted_price = m_exchange_rates.convert(price,
            order->get_info().m_fields.m_currency, converted_fields.m_currency);
        } catch(const CurrencyPairNotFoundException&) {
          return;
        }
        buying_power_model.submit(
          order->get_info().m_id, converted_fields, converted_price);
      });
    order->get_publisher().Monitor(
      buying_power_entry.m_execution_report_queue.GetWriter());
  }

  template<typename A, typename M>
  void BuyingPowerCheck<A, M>::reject(const OrderInfo& info) {
    auto& buying_power_entry = load_buying_power_entry(info.m_fields.m_account);
    Beam::Threading::With(
      buying_power_entry.m_buying_power_model, [&] (auto& buying_power_model) {
        if(!buying_power_model.has_order(info.m_id)) {
          return;
        }
        auto report = ExecutionReport();
        report.m_id = info.m_id;
        report.m_status = OrderStatus::REJECTED;
        buying_power_model.update(report);
      });
  }

  template<typename A, typename M>
  BboQuote BuyingPowerCheck<A, M>::load_bbo_quote(const Security& security) {
    auto publisher = m_bbo_quotes.GetOrInsert(security, [&] {
      auto publisher = std::make_shared<Beam::StateQueue<BboQuote>>();
      MarketDataService::query_real_time_with_snapshot(
        security, *m_market_data_client, publisher);
      return publisher;
    });
    try {
      return publisher->Peek();
    } catch(const Beam::PipeBrokenException&) {
      m_bbo_quotes.Erase(security);
      BOOST_THROW_EXCEPTION(
        OrderSubmissionCheckException("No BBO quote available."));
    }
  }

  template<typename A, typename M>
  Money BuyingPowerCheck<A, M>::get_expected_price(const OrderFields& fields) {
    auto bbo = load_bbo_quote(fields.m_security);
    if(fields.m_type == OrderType::LIMIT) {
      if(fields.m_price <= Money::ZERO) {
        BOOST_THROW_EXCEPTION(OrderSubmissionCheckException("Invalid price."));
      } else if(fields.m_side == Side::ASK) {
        return std::max(bbo.m_bid.m_price, fields.m_price);
      } else {
        return std::min(bbo.m_ask.m_price, fields.m_price);
      }
    } else if(fields.m_side == Side::ASK) {
      return bbo.m_bid.m_price;
    } else {
      return bbo.m_ask.m_price;
    }
  }

  template<typename A, typename M>
  typename BuyingPowerCheck<A, M>::BuyingPowerEntry&
      BuyingPowerCheck<A,M>::load_buying_power_entry(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto& entry = *m_buying_power_entries.GetOrInsert(account, [&] {
      auto entry = std::make_shared<BuyingPowerEntry>();
      m_administration_client->get_risk_parameters_publisher(account).Monitor(
        entry->m_risk_parameters_queue);
      return entry;
    });
    entry.m_risk_parameters_queue->Peek();
    return entry;
  }
}

#endif
