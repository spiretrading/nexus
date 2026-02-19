#ifndef NEXUS_BUYING_POWER_CHECK_HPP
#define NEXUS_BUYING_POWER_CHECK_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/MultiQueueWriter.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Accounting/BuyingPowerModel.hpp"
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/MarketDataService/TickerQuery.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheck.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheckException.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

namespace Nexus {

  /**
   * Performs a buying power check on an Order submission.
   * @param <A> The type of AdministrationClient used to get the RiskParameters.
   * @param <M> The type of MarketDataClient used to price Orders for buying
   *        power checks.
   */
  template<typename A, typename M> requires
    IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>>
  class BuyingPowerCheck : public OrderSubmissionCheck {
    public:

      /** The type of AdministrationClient used to get the RiskParameters. */
      using AdministrationClient = Beam::dereference_t<A>;

      /**
       * The type of MarketDataClient used to price Orders for buying power
       * checks.
       */
      using MarketDataClient = Beam::dereference_t<M>;

      /**
       * Constructs a BuyingPowerCheck.
       * @param exchange_rates The list of ExchangeRates.
       * @param administration_client Initializes the AdministrationClient.
       * @param market_data_client Initializes the MarketDataClient.
       */
      template<Beam::Initializes<A> AF, Beam::Initializes<M> MF>
      BuyingPowerCheck(const ExchangeRateTable& exchange_rates,
        AF&& administration_client, MF&& market_data_client);

      void submit(const OrderInfo& info) override;
      void add(const std::shared_ptr<Order>& order) override;
      void reject(const OrderInfo& info) override;

    private:
      struct BuyingPowerEntry {
        Beam::Sync<BuyingPowerModel> m_buying_power_model;
        std::shared_ptr<Beam::StateQueue<RiskParameters>>
          m_risk_parameters_queue;
        Beam::MultiQueueWriter<ExecutionReport> m_execution_report_queue;
        Beam::SynchronizedUnorderedMap<OrderId, Asset> m_currencies;

        BuyingPowerEntry();
      };
      ExchangeRateTable m_exchange_rates;
      Beam::local_ptr_t<A> m_administration_client;
      Beam::local_ptr_t<M> m_market_data_client;
      Beam::SynchronizedUnorderedMap<Beam::DirectoryEntry,
        std::shared_ptr<BuyingPowerEntry>> m_buying_power_entries;
      Beam::SynchronizedUnorderedMap<
        Ticker, std::shared_ptr<Beam::StateQueue<BboQuote>>> m_bbo_quotes;
      std::vector<Beam::RoutineHandler> m_query_routines;

      BboQuote load_bbo_quote(const Ticker& ticker);
      Money get_expected_price(const OrderFields& fields);
      BuyingPowerEntry& load_buying_power_entry(
        const Beam::DirectoryEntry& account);
  };

  /**
   * Makes a BuyingPowerCheck.
   * @param exchange_rates The list of ExchangeRates.
   * @param administration_client Initializes the AdministrationClient.
   * @param market_data_client Initializes the MarketDataClient.
   */
  template<IsAdministrationClient A, IsMarketDataClient M>
  auto make_buying_power_check(const ExchangeRateTable& exchange_rates,
      A&& administration_client, M&& market_data_client) {
    return std::make_unique<
      BuyingPowerCheck<std::remove_reference_t<A>, std::remove_reference_t<M>>>(
        exchange_rates, std::forward<A>(administration_client),
        std::forward<M>(market_data_client));
  }

  template<typename A, typename M> requires
    IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>>
  BuyingPowerCheck<A, M>::BuyingPowerEntry::BuyingPowerEntry()
    : m_risk_parameters_queue(
        std::make_shared<Beam::StateQueue<RiskParameters>>()) {}

  template<typename A, typename M> requires
    IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>>
  template<Beam::Initializes<A> AF, Beam::Initializes<M> MF>
  BuyingPowerCheck<A, M>::BuyingPowerCheck(
    const ExchangeRateTable& exchange_rates, AF&& administration_client,
    MF&& market_data_client)
    : m_exchange_rates(exchange_rates),
      m_administration_client(std::forward<AF>(administration_client)),
      m_market_data_client(std::forward<MF>(market_data_client)) {}

  template<typename A, typename M> requires
    IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>>
  void BuyingPowerCheck<A, M>::submit(const OrderInfo& info) {
    auto& fields = info.m_fields;
    auto price = get_expected_price(fields);
    auto& buying_power_entry = load_buying_power_entry(fields.m_account);
    Beam::with(
      buying_power_entry.m_buying_power_model, [&] (auto& buying_power_model) {
        auto risk_parameters =
          buying_power_entry.m_risk_parameters_queue->peek();
        while(auto report =
            buying_power_entry.m_execution_report_queue.try_pop()) {
          if(report->m_last_quantity != 0) {
            auto currency =
              buying_power_entry.m_currencies.try_load(report->m_id);
            if(!currency) {
              boost::throw_with_location(
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
          boost::throw_with_location(
            OrderSubmissionCheckException("Currency not recognized."));
        }
        buying_power_entry.m_currencies.insert(info.m_id, fields.m_currency);
        auto updated_buying_power = buying_power_model.submit(
          info.m_id, converted_fields, converted_price);
        if(updated_buying_power > risk_parameters.m_buying_power) {
          auto report = ExecutionReport();
          report.m_id = info.m_id;
          report.m_status = OrderStatus::REJECTED;
          buying_power_model.update(report);
          boost::throw_with_location(OrderSubmissionCheckException(
            "Order exceeds available buying power."));
        }
      });
  }

  template<typename A, typename M> requires
    IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>>
  void BuyingPowerCheck<A, M>::add(const std::shared_ptr<Order>& order) {
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
    Beam::with(
      buying_power_entry.m_buying_power_model, [&] (auto& buying_power_model) {
        if(buying_power_model.has_order(order->get_info().m_id)) {
          return;
        }
        buying_power_entry.m_currencies.insert(
          order->get_info().m_id, order->get_info().m_fields.m_currency);
        auto converted_fields = order->get_info().m_fields;
        converted_fields.m_currency =
          buying_power_entry.m_risk_parameters_queue->peek().m_currency;
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
    order->get_publisher().monitor(
      buying_power_entry.m_execution_report_queue.get_writer());
  }

  template<typename A, typename M> requires
    IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>>
  void BuyingPowerCheck<A, M>::reject(const OrderInfo& info) {
    auto& buying_power_entry = load_buying_power_entry(info.m_fields.m_account);
    Beam::with(
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

  template<typename A, typename M> requires
    IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>>
  BboQuote BuyingPowerCheck<A, M>::load_bbo_quote(const Ticker& ticker) {
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
        OrderSubmissionCheckException("No BBO quote available."));
    }
  }

  template<typename A, typename M> requires
    IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>>
  Money BuyingPowerCheck<A, M>::get_expected_price(const OrderFields& fields) {
    auto bbo = load_bbo_quote(fields.m_ticker);
    if(fields.m_type == OrderType::LIMIT) {
      if(fields.m_price <= Money::ZERO) {
        boost::throw_with_location(
          OrderSubmissionCheckException("Invalid price."));
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

  template<typename A, typename M> requires
    IsAdministrationClient<Beam::dereference_t<A>> &&
      IsMarketDataClient<Beam::dereference_t<M>>
  typename BuyingPowerCheck<A, M>::BuyingPowerEntry&
      BuyingPowerCheck<A,M>::load_buying_power_entry(
        const Beam::DirectoryEntry& account) {
    auto& entry = *m_buying_power_entries.get_or_insert(account, [&] {
      auto entry = std::make_shared<BuyingPowerEntry>();
      m_administration_client->get_risk_parameters_publisher(account).monitor(
        entry->m_risk_parameters_queue);
      return entry;
    });
    entry.m_risk_parameters_queue->peek();
    return entry;
  }
}

#endif
