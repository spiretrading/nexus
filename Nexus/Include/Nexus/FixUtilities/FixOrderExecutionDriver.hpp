#ifndef NEXUS_FIX_ORDER_EXECUTION_DRIVER_HPP
#define NEXUS_FIX_ORDER_EXECUTION_DRIVER_HPP
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Sync.hpp>
#include <boost/noncopyable.hpp>
#include <quickfix/Application.h>
#include <quickfix/FileStore.h>
#include <quickfix/FileLog.h>
#include <quickfix/Session.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>
#include "Nexus/FixUtilities/FixApplication.hpp"
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Nexus/OrderExecutionService/OrderUnrecoverableException.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus::FixUtilities {

  /**
   * Stores the details of a single FIX Application used by a
   * FixOrderExecutionDriver.
   */
  struct FixApplicationEntry {

    /** The path to the configuration file. */
    std::string m_configPath;

    /** The list of destinations the FIX Application services. */
    std::vector<std::string> m_destinations;

    /** The FIX Application used to service Order entries. */
    std::shared_ptr<FixApplication> m_application;
  };

  /** Implements the OrderExecutionDriver using the FIX protocol. */
  class FixOrderExecutionDriver : private boost::noncopyable {
    public:

      /**
       * Constructs a FixOrderExecutionDriver.
       * @param fixApplicationEntries The list of FIX Application entries
       *        servicing Order submissions.
       */
      FixOrderExecutionDriver(
        const std::vector<FixApplicationEntry>& fixApplicationEntries);

      ~FixOrderExecutionDriver();

      const OrderExecutionService::Order& Recover(
        const OrderExecutionService::SequencedAccountOrderRecord& orderRecord);

      const OrderExecutionService::Order& Submit(
        const OrderExecutionService::OrderInfo& info);

      void Cancel(const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId);

      void Update(const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId,
        const OrderExecutionService::ExecutionReport& executionReport);

      void Close();

    private:
      struct Application {
        std::shared_ptr<FixApplication> m_application;
        std::string m_configPath;
        bool m_isConnected;
        std::optional<FIX::SessionSettings> m_settings;
        std::optional<FIX::FileStoreFactory> m_storeFactory;
        std::optional<FIX::FileLogFactory> m_logFactory;
        std::optional<FIX::SocketInitiator> m_initiator;

        Application(std::shared_ptr<FixApplication> application,
          std::string configPath);
      };
      std::unordered_map<std::string, std::shared_ptr<Application>>
        m_fixApplications;
      Beam::Threading::Sync<std::unordered_map<OrderExecutionService::OrderId,
        std::shared_ptr<Application>>> m_orderIdToFixApplication;
      Beam::Threading::Sync<std::vector<
        std::unique_ptr<OrderExecutionService::PrimitiveOrder>>> m_orders;
      Beam::IO::OpenState m_openState;
  };

  inline FixOrderExecutionDriver::Application::Application(
    std::shared_ptr<FixApplication> application, std::string configPath)
    : m_application(std::move(application)),
      m_configPath(std::move(configPath)),
      m_isConnected(false) {}

  inline FixOrderExecutionDriver::FixOrderExecutionDriver(
      const std::vector<FixApplicationEntry>& fixApplicationEntries) {
    for(auto& entry : fixApplicationEntries) {
      auto application = std::make_shared<Application>(entry.m_application,
        entry.m_configPath);
      for(const auto& destination : entry.m_destinations) {
        m_fixApplications.insert(std::make_pair(destination, application));
      }
    }
    try {
      auto initializedApplications =
        std::unordered_set<std::shared_ptr<Application>>();
      for(auto& fixApplication : m_fixApplications) {
        if(!initializedApplications.insert(fixApplication.second).second) {
          continue;
        }
        try {
          auto& entry = *(fixApplication.second);
          entry.m_settings.emplace(entry.m_configPath);
          if(entry.m_settings->getSessions().size() != 1) {
            BOOST_THROW_EXCEPTION(std::runtime_error(
              "Only one session per application is permitted."));
          }
          auto sessionId = *entry.m_settings->getSessions().begin();
          entry.m_storeFactory.emplace(*entry.m_settings);
          entry.m_logFactory.emplace(*entry.m_settings);
          entry.m_initiator.emplace(*entry.m_application,
            *entry.m_storeFactory, *entry.m_settings, *entry.m_logFactory);
          entry.m_application->SetSessionSettings(sessionId, *entry.m_settings);
          for(auto& sessionId : entry.m_settings->getSessions()) {
            auto session = FIX::Session::lookupSession(sessionId);
            if(session != nullptr) {
              auto dataDictionaryProvider =
                session->getDataDictionaryProvider();
              auto sessionDataDictionary =
                dataDictionaryProvider.getSessionDataDictionary(
                sessionId.getBeginString());
              sessionDataDictionary.checkFieldsOutOfOrder(false);
              sessionDataDictionary.checkFieldsHaveValues(false);
              dataDictionaryProvider.addTransportDataDictionary(
                sessionId.getBeginString(),
                std::make_shared<FIX::DataDictionary>(sessionDataDictionary));
              auto applicationDataDictionary =
                dataDictionaryProvider.getApplicationDataDictionary(
                FIX::Message::toApplVerID(sessionId.getBeginString()));
              applicationDataDictionary.checkFieldsOutOfOrder(false);
              applicationDataDictionary.checkFieldsHaveValues(false);
              dataDictionaryProvider.addApplicationDataDictionary(
                FIX::Message::toApplVerID(sessionId.getBeginString()),
                std::make_shared<FIX::DataDictionary>(
                applicationDataDictionary));
              session->setDataDictionaryProvider(dataDictionaryProvider);
            }
          }
          entry.m_initiator->start();
          entry.m_isConnected = true;
        } catch(const std::exception& e) {
          std::cerr << "Error: " << fixApplication.first << ": " << e.what();
        }
      }
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  inline FixOrderExecutionDriver::~FixOrderExecutionDriver() {
    Close();
  }

  inline const OrderExecutionService::Order& FixOrderExecutionDriver::Recover(
      const OrderExecutionService::SequencedAccountOrderRecord& orderRecord) {
    auto fixApplicationEntryIterator = m_fixApplications.find(
      (*orderRecord)->m_info.m_fields.m_destination);
    if(fixApplicationEntryIterator == m_fixApplications.end()) {
      BOOST_THROW_EXCEPTION(
        OrderExecutionService::OrderUnrecoverableException());
    }
    auto fixApplicationEntry = fixApplicationEntryIterator->second;
    auto& order = fixApplicationEntry->m_application->Recover(orderRecord);
    Beam::Threading::With(m_orderIdToFixApplication,
      [&] (auto& orderIdToFixApplication) {
        orderIdToFixApplication.insert(
          std::make_pair((*orderRecord)->m_info.m_orderId,
          fixApplicationEntry));
      });
    return order;
  }

  inline const OrderExecutionService::Order& FixOrderExecutionDriver::Submit(
      const OrderExecutionService::OrderInfo& info) {
    auto fixApplicationEntryIterator = m_fixApplications.find(
      info.m_fields.m_destination);
    if(fixApplicationEntryIterator == m_fixApplications.end()) {
      auto order = OrderExecutionService::BuildRejectedOrder(info,
        "Destination [" + info.m_fields.m_destination + "] not available");
      auto& submittedOrder = *order;
      Beam::Threading::With(m_orders,
        [&] (auto& orders) {
          orders.push_back(std::move(order));
        });
      return submittedOrder;
    }
    auto fixApplicationEntry = fixApplicationEntryIterator->second;
    const auto& order = fixApplicationEntry->m_application->Submit(info);
    Beam::Threading::With(m_orderIdToFixApplication,
      [&] (auto& orderIdToFixApplication) {
        orderIdToFixApplication.insert(
          std::make_pair(info.m_orderId, fixApplicationEntry));
      });
    return order;
  }

  inline void FixOrderExecutionDriver::Cancel(
      const OrderExecutionService::OrderExecutionSession& session,
      OrderExecutionService::OrderId orderId) {
    auto fixApplicationEntry = Beam::Threading::With(m_orderIdToFixApplication,
      [&] (auto& orderIdToFixApplication) {
        auto entryIterator = orderIdToFixApplication.find(orderId);
        if(entryIterator == orderIdToFixApplication.end()) {
          return std::shared_ptr<Application>();
        }
        return entryIterator->second;
      });
    if(fixApplicationEntry) {
      fixApplicationEntry->m_application->Cancel(session, orderId);
    }
  }

  inline void FixOrderExecutionDriver::Update(
      const OrderExecutionService::OrderExecutionSession& session,
      OrderExecutionService::OrderId orderId,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto fixApplicationEntry = Beam::Threading::With(m_orderIdToFixApplication,
      [&] (const auto& orderIdToFixApplication) {
        auto entryIterator = orderIdToFixApplication.find(orderId);
        if(entryIterator == orderIdToFixApplication.end()) {
          return std::shared_ptr<Application>{};
        }
        return entryIterator->second;
      });
    if(fixApplicationEntry) {
      fixApplicationEntry->m_application->Update(session, orderId,
        executionReport);
    }
  }

  inline void FixOrderExecutionDriver::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_openState.Close();
    for(auto& fixApplication : m_fixApplications) {
      auto& entry = *(fixApplication.second);
      entry.m_initiator->stop();
      entry.m_isConnected = false;
    }
  }
}

#endif
