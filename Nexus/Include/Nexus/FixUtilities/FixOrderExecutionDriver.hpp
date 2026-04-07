#ifndef NEXUS_FIX_ORDER_EXECUTION_DRIVER_HPP
#define NEXUS_FIX_ORDER_EXECUTION_DRIVER_HPP
#include <optional>
#include <vector>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/OpenState.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include <quickfix/Application.h>
#include <quickfix/FileStore.h>
#include <quickfix/FileLog.h>
#include <quickfix/Session.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>
#include "Nexus/FixUtilities/FixApplication.hpp"
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderUnrecoverableException.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus {

  /**
   * Stores the details of a single FIX Application used by a
   * FixOrderExecutionDriver.
   */
  struct FixApplicationEntry {

    /** The FIX session settings. */
    FIX::SessionSettings m_settings;

    /** The list of destinations the FIX Application services. */
    std::vector<std::string> m_destinations;

    /** The FIX Application used to service Order entries. */
    std::shared_ptr<FixApplication> m_application;
  };

  /** Implements the OrderExecutionDriver using the FIX protocol. */
  class FixOrderExecutionDriver {
    public:

      /**
       * Constructs a FixOrderExecutionDriver.
       * @param entries The list of FIX Application entries servicing Order
       *        submissions.
       */
      explicit FixOrderExecutionDriver(
        const std::vector<FixApplicationEntry>& entries);

      ~FixOrderExecutionDriver();

      std::shared_ptr<Order> recover(const SequencedAccountOrderRecord& record);
      void add(const std::shared_ptr<Order>& order);
      std::shared_ptr<Order> submit(const OrderInfo& info);
      void cancel(const OrderExecutionSession& session, OrderId id);
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report);
      void close();

    private:
      struct Application {
        std::shared_ptr<FixApplication> m_application;
        bool m_is_connected;
        FIX::SessionSettings m_settings;
        std::optional<FIX::FileStoreFactory> m_store_factory;
        std::optional<FIX::FileLogFactory> m_log_factory;
        std::optional<FIX::SocketInitiator> m_initiator;

        Application(std::shared_ptr<FixApplication> application,
          FIX::SessionSettings settings);
      };
      std::unordered_map<std::string, std::shared_ptr<Application>>
        m_applications;
      Beam::SynchronizedUnorderedMap<OrderId, std::shared_ptr<Application>>
        m_id_to_application;
      Beam::OpenState m_open_state;

      FixOrderExecutionDriver(const FixOrderExecutionDriver&) = delete;
      FixOrderExecutionDriver& operator =(
        const FixOrderExecutionDriver&) = delete;
  };

  inline FixOrderExecutionDriver::Application::Application(
    std::shared_ptr<FixApplication> application, FIX::SessionSettings settings)
    : m_application(std::move(application)),
      m_is_connected(false),
      m_settings(std::move(settings)) {}

  inline FixOrderExecutionDriver::FixOrderExecutionDriver(
      const std::vector<FixApplicationEntry>& entries) {
    for(auto& entry : entries) {
      auto application =
        std::make_shared<Application>(entry.m_application, entry.m_settings);
      for(auto& destination : entry.m_destinations) {
        m_applications.insert(std::pair(destination, application));
      }
    }
    try {
      auto initialized_applications =
        std::unordered_set<std::shared_ptr<Application>>();
      for(auto& application : m_applications) {
        if(!initialized_applications.insert(application.second).second) {
          continue;
        }
        try {
          auto& entry = *(application.second);
          if(entry.m_settings.getSessions().size() != 1) {
            boost::throw_with_location(std::runtime_error(
              "Only one session per application is permitted."));
          }
          auto session_id = *entry.m_settings.getSessions().begin();
          entry.m_store_factory.emplace(entry.m_settings);
          entry.m_log_factory.emplace(entry.m_settings);
          entry.m_initiator.emplace(*entry.m_application,
            *entry.m_store_factory, entry.m_settings, *entry.m_log_factory);
          entry.m_application->set_session_settings(
            session_id, entry.m_settings);
          for(auto& session_id : entry.m_settings.getSessions()) {
            if(auto session = FIX::Session::lookupSession(session_id)) {
              auto dictionary_provider = session->getDataDictionaryProvider();
              auto session_dictionary =
                dictionary_provider.getSessionDataDictionary(
                  session_id.getBeginString());
              session_dictionary.checkFieldsOutOfOrder(false);
              session_dictionary.checkFieldsHaveValues(false);
              dictionary_provider.addTransportDataDictionary(
                session_id.getBeginString(),
                std::make_shared<FIX::DataDictionary>(session_dictionary));
              auto application_dictionary =
                dictionary_provider.getApplicationDataDictionary(
                  FIX::Message::toApplVerID(session_id.getBeginString()));
              application_dictionary.checkFieldsOutOfOrder(false);
              application_dictionary.checkFieldsHaveValues(false);
              dictionary_provider.addApplicationDataDictionary(
                FIX::Message::toApplVerID(session_id.getBeginString()),
                std::make_shared<FIX::DataDictionary>(
                  application_dictionary));
              session->setDataDictionaryProvider(dictionary_provider);
            }
          }
          entry.m_initiator->start();
          entry.m_is_connected = true;
        } catch(const std::exception& e) {
          std::cerr << "Error: " << application.first << ": " << e.what();
        }
      }
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  inline FixOrderExecutionDriver::~FixOrderExecutionDriver() {
    close();
  }

  inline std::shared_ptr<Order> FixOrderExecutionDriver::recover(
      const SequencedAccountOrderRecord& record) {
    auto i = m_applications.find((*record)->m_info.m_fields.m_destination);
    if(i == m_applications.end()) {
      boost::throw_with_location(OrderUnrecoverableException(
        "FIX application for given destination not found: [" +
        (*record)->m_info.m_fields.m_destination + "], " +
        boost::lexical_cast<std::string>((*record)->m_info.m_id)));
    }
    auto entry = i->second;
    auto order = entry->m_application->recover(record);
    m_id_to_application.insert((*record)->m_info.m_id, entry);
    return order;
  }

  inline void FixOrderExecutionDriver::add(
    const std::shared_ptr<Order>& order) {}

  inline std::shared_ptr<Order> FixOrderExecutionDriver::submit(
      const OrderInfo& info) {
    auto i = m_applications.find(info.m_fields.m_destination);
    if(i == m_applications.end()) {
      return make_rejected_order(info,
        "Destination [" + info.m_fields.m_destination + "] not available");
    }
    auto entry = i->second;
    auto order = entry->m_application->submit(info);
    m_id_to_application.insert(info.m_id, entry);
    return order;
  }

  inline void FixOrderExecutionDriver::cancel(
      const OrderExecutionSession& session, OrderId id) {
    if(auto entry = m_id_to_application.find(id)) {
      (*entry)->m_application->cancel(session, id);
    }
  }

  inline void FixOrderExecutionDriver::update(
      const OrderExecutionSession& session, OrderId id,
      const ExecutionReport& report) {
    if(auto entry = m_id_to_application.find(id)) {
      (*entry)->m_application->update(session, id, report);
    }
  }

  inline void FixOrderExecutionDriver::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    for(auto& application : m_applications) {
      auto& entry = *(application.second);
      entry.m_initiator->stop();
      entry.m_is_connected = false;
    }
    m_open_state.close();
  }
}

#endif
