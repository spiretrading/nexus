#ifndef NEXUS_RISK_CONTROLLER_HPP
#define NEXUS_RISK_CONTROLLER_HPP
#include <memory>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queues/TablePublisher.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Threading/Timer.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/RiskStateProcessor.hpp"
#include "Nexus/RiskService/RiskTransitionProcessor.hpp"

namespace Nexus::RiskService {

  /** Represents an entry in a RiskState table. */
  using RiskStateEntry = Beam::TableEntry<Beam::ServiceLocator::DirectoryEntry,
    RiskState>;

  /**
   * Instantiates RiskStateProcessors and RiskTransitionProcessors for all
   * accounts received from a queue and updates both processors based on Orders
   * submitted and market data. Portfolio's are valued using the BboQuote
   * published by a MarketDataClient and Orders published by an
   * OrderExecutionPublisher.
   * @param <A> The type of AdministrationClient used to load an account's
   *        RiskParameters.
   * @param <M> The type of MarketDataClient to use.
   * @param <O> The type of OrderExecutionClient to use.
   * @param <R> The type of Timer to use to transition from CLOSED_ORDERS to
   *        DISABLED.
   * @param <T> The type of TimeClient to use.
   */
  template<typename A, typename M, typename O, typename R, typename T>
  class RiskController : private boost::noncopyable {
    public:

      /**
       * The type of AdministrationClient used to load an account's
       * RiskParameters.
       */
      using AdministrationClient = Beam::GetTryDereferenceType<A>;

      /** The type of MarketDataClient to use. */
      using MarketDataClient = Beam::GetTryDereferenceType<M>;

      /** The type of OrderExecutionClient to use. */
      using OrderExecutionClient = Beam::GetTryDereferenceType<O>;

      /** The type of TransitionTimer to use. */
      using TransitionTimer = Beam::GetTryDereferenceType<R>;

      /** The type of TimeClient to use. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /**
       * Constructs a RiskController.
       * @param accounts The Queue publishing the accounts to control.
       * @param administrationClient Initializes the AdministrationClient.
       * @param marketDataClient Initializes the MarketDataClient.
       * @param orderExecutionClient Initializes the OrderExecutionClient.
       * @param transitionTimer Initializes the transition Timer.
       * @param timeClient Initializes the TimeClient.
       * @param exchangeRates The list of exchange rates.
       * @param destinations The destination database used to flatten positions.
       */
      template<typename AF, typename MF, typename OF, typename RF, typename TF>
      RiskController(
        std::shared_ptr<Beam::QueueWriter<Beam::ServiceLocator::DirectoryEntry>>
        accounts, AF&& administrationClient, MF&& marketDataClient,
        OF&& orderExecutionClient, RF&& transitionTimer, TF&& timeClient,
        const std::vector<ExchangeRate>& exchangeRates,
        const DestinationDatabase& destinations);

      /** Returns the object used to publish RiskState updates. */
      const Beam::Publisher<RiskStateEntry>& GetRiskStatePublisher() const;

      /** Returns the object used to publish Inventory updates. */
      const RiskPortfolioUpdatePublisher& GetInventoryPublisher() const;
  };
}

#endif
