#ifndef NEXUS_RISKTRANSITIONCONTROLLER_HPP
#define NEXUS_RISKTRANSITIONCONTROLLER_HPP
#include <unordered_map>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Queues/QueueReaderPublisher.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/RiskTransitionTracker.hpp"

namespace Nexus {
namespace RiskService {

  /*! \class RiskTransitionController
      \brief Instantiates and updates RiskTransitionTrackers based on signals
             received from an AdministrationClient and OrderExecutionClient.
      \tparam AdministrationClientType The type of AdministrationClient used to
              receive RiskState updates.
      \tparam OrderExecutionClientType The type of OrderExecutionClient used to
              monitor Order executions.
   */
  template<typename AdministrationClientType, typename OrderExecutionClientType>
  class RiskTransitionController : private boost::noncopyable {
    public:

      //! The type of AdministrationClient used to receive RiskState updates.
      using AdministrationClient =
        Beam::GetTryDereferenceType<AdministrationClientType>;

      //! The type of OrderExecutionClient used to monitor Order executions.
      using OrderExecutionClient =
        Beam::GetTryDereferenceType<OrderExecutionClientType>;

      //! Constructs a RiskTransitionController.
      /*!
        \param orderQueue The Queue publishing Orders to monitor.
        \param administrationClient Initializes the AdministrationClient.
        \param orderExecutionClient Initializes the OrderExecutionClient.
        \param destinations The database of available Destinations.
        \param markets The database of available Markets.
      */
      template<typename AdministrationClientForward,
        typename OrderExecutionClientForward>
      RiskTransitionController(const std::shared_ptr<Beam::QueueReader<
        const OrderExecutionService::Order*>>& orderQueue,
        AdministrationClientForward&& administrationClient,
        OrderExecutionClientForward&& orderExecutionClient,
        const DestinationDatabase& destinations, const MarketDatabase& markets);

      ~RiskTransitionController();

      void Open();

      void Close();

    private:
      Beam::QueueReaderPublisher<const OrderExecutionService::Order*>
        m_orderPublisher;
      Beam::GetOptionalLocalPtr<AdministrationClientType>
        m_administrationClient;
      Beam::GetOptionalLocalPtr<OrderExecutionClientType>
        m_orderExecutionClient;
      DestinationDatabase m_destinations;
      MarketDatabase m_markets;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
        Beam::LocalPtr<RiskTransitionTracker<OrderExecutionClient*>>>
        m_trackers;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void Shutdown();
      void OnRiskState(RiskTransitionTracker<OrderExecutionClient*>& tracker,
        const RiskState& riskState);
      void OnOrderSubmission(const OrderExecutionService::Order* order);
      void OnExecutionReport(
        RiskTransitionTracker<OrderExecutionClient*>& tracker,
        const OrderExecutionService::ExecutionReport& report);
  };

  template<typename AdministrationClientType, typename OrderExecutionClientType>
  template<typename AdministrationClientForward,
    typename OrderExecutionClientForward>
  RiskTransitionController<AdministrationClientType, OrderExecutionClientType>::
      RiskTransitionController(const std::shared_ptr<Beam::QueueReader<
      const OrderExecutionService::Order*>>& orderQueue,
      AdministrationClientForward&& administrationClient,
      OrderExecutionClientForward&& orderExecutionClient,
      const DestinationDatabase& destinations, const MarketDatabase& markets)
      : m_orderPublisher(orderQueue),
        m_administrationClient(std::forward<AdministrationClientForward>(
          administrationClient)),
        m_orderExecutionClient(std::forward<OrderExecutionClientForward>(
          orderExecutionClient)),
        m_destinations(destinations),
        m_markets(markets) {}

  template<typename AdministrationClientType, typename OrderExecutionClientType>
  RiskTransitionController<AdministrationClientType, OrderExecutionClientType>::
      ~RiskTransitionController() {
    Close();
  }

  template<typename AdministrationClientType, typename OrderExecutionClientType>
  void RiskTransitionController<AdministrationClientType,
      OrderExecutionClientType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_administrationClient->Open();
      m_orderExecutionClient->Open();
      m_orderPublisher.Monitor(
        m_tasks.GetSlot<const OrderExecutionService::Order*>(std::bind(
        &RiskTransitionController::OnOrderSubmission, this,
        std::placeholders::_1)));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename AdministrationClientType, typename OrderExecutionClientType>
  void RiskTransitionController<AdministrationClientType,
      OrderExecutionClientType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename AdministrationClientType, typename OrderExecutionClientType>
  void RiskTransitionController<AdministrationClientType,
      OrderExecutionClientType>::Shutdown() {
    m_tasks.Break();
    m_openState.SetClosed();
  }

  template<typename AdministrationClientType, typename OrderExecutionClientType>
  void RiskTransitionController<AdministrationClientType,
      OrderExecutionClientType>::OnRiskState(
      RiskTransitionTracker<OrderExecutionClient*>& tracker,
      const RiskState& riskState) {
    tracker.Update(riskState);
  }

  template<typename AdministrationClientType, typename OrderExecutionClientType>
  void RiskTransitionController<AdministrationClientType,
      OrderExecutionClientType>::OnOrderSubmission(
      const OrderExecutionService::Order* order) {
    auto& account = order->GetInfo().m_fields.m_account;
    auto trackerIterator = m_trackers.find(account);
    if(trackerIterator == m_trackers.end()) {
      trackerIterator = m_trackers.emplace(account, Beam::Initialize(
        account, *&m_orderExecutionClient, m_destinations, m_markets)).first;
      m_administrationClient->GetRiskStatePublisher(account).Monitor(
        m_tasks.GetSlot<RiskState>(std::bind(
        &RiskTransitionController::OnRiskState, this,
        std::ref(*trackerIterator->second), std::placeholders::_1)));
    }
    auto& tracker = *trackerIterator->second;
    tracker.Add(*order);
    order->GetPublisher().Monitor(
      m_tasks.GetSlot<OrderExecutionService::ExecutionReport>(std::bind(
      &RiskTransitionController::OnExecutionReport, this, std::ref(tracker),
      std::placeholders::_1)));
  }

  template<typename AdministrationClientType, typename OrderExecutionClientType>
  void RiskTransitionController<AdministrationClientType,
      OrderExecutionClientType>::OnExecutionReport(
      RiskTransitionTracker<OrderExecutionClient*>& tracker,
      const OrderExecutionService::ExecutionReport& executionReport) {
    tracker.Update(executionReport);
  }
}
}

#endif
