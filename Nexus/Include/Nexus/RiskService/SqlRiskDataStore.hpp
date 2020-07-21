#ifndef NEXUS_SQL_RISK_DATA_STORE_HPP
#define NEXUS_SQL_RISK_DATA_STORE_HPP
#include <memory>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/noncopyable.hpp>
#include <Viper/Transaction.hpp>
#include "Nexus/RiskService/RiskDataStore.hpp"
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/SqlDefinitions.hpp"

namespace Nexus::RiskService {

  /**
   * Implements a RiskDataStore backed by an SQL database.
   * @param <C> The SQL connection to use.
   */
  template<typename C>
  class SqlRiskDataStore : private boost::noncopyable {
    public:

      /** The SQL connection to use. */
      using Connection = C;

      /**
       * Constructs an SqlRiskDataStore.
       * @param connection The SQL connection to use.
       */
      explicit SqlRiskDataStore(std::unique_ptr<Connection> connection);

      ~SqlRiskDataStore();

      PositionSnapshot LoadPositionSnapshot(
        const Beam::ServiceLocator::DirectoryEntry& account);

      void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const PositionSnapshot& snapshot);

      void Open();

      void Close();

    private:
      std::unique_ptr<Connection> m_connection;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void Shutdown();
  };

  template<typename C>
  SqlRiskDataStore<C>::SqlRiskDataStore(std::unique_ptr<Connection> connection)
    : m_connection(std::move(connection)) {}

  template<typename C>
  SqlRiskDataStore<C>::~SqlRiskDataStore() {
    Close();
  }

  template<typename C>
  PositionSnapshot SqlRiskDataStore<C>::LoadPositionSnapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
/*
    auto rows = std::vector<ComplianceRuleEntriesRow>();
    {
      auto lock = std::lock_guard(m_mutex);
      try {
        m_connection->execute(Viper::select(GetComplianceRuleEntriesRow(),
          "compliance_rule_entries", std::back_inserter(rows)));
      } catch(const Viper::ExecuteException& e) {
        BOOST_THROW_EXCEPTION(ComplianceRuleDataStoreException(e.what()));
      }
    }
    auto entries = std::vector<ComplianceRuleEntry>();
    std::transform(rows.begin(), rows.end(), std::back_inserter(entries),
      &ConvertRow);
    return entries;
*/
    return {};
  }

  template<typename C>
  void SqlRiskDataStore<C>::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const PositionSnapshot& snapshot) {
    m_tasks.Push([&] {
      Viper::transaction(*m_connection, [&] {
        m_connection->execute(Viper::insert(GetPositionEntriesRow(),
          "position_entries", boost::iterators::make_transform_iterator(
          snapshot.m_positions.begin(), ConvertPositionSnapshotPositions(
          account)), snapshot.m_positions.end()));
      });
    });
/*
    auto row = ComplianceRuleEntriesRow{entry.GetId(),
      entry.GetDirectoryEntry(), entry.GetState(), entry.GetSchema().GetName(),
      std::move(parameterBuffer)};
    try {
      auto lock = std::lock_guard(m_mutex);
      m_connection->execute(Viper::upsert(GetComplianceRuleEntriesRow(),
        "compliance_rule_entries", &row));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ComplianceRuleDataStoreException(e.what()));
    }
*/
  }

  template<typename C>
  void SqlRiskDataStore<C>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_connection->open();
      m_connection->execute(Viper::create_if_not_exists(
        GetPositionEntriesRow(), "position_entries"));
      m_connection->execute(Viper::create_if_not_exists(
        GetPositionSequencesRow(), "position_sequences"));
      m_connection->execute(Viper::create_if_not_exists(
        GetPositionExcludedOrdersRow(), "position_excluded_orders"));
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename C>
  void SqlRiskDataStore<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C>
  void SqlRiskDataStore<C>::Shutdown() {
    m_tasks.Break();
    m_tasks.Wait();
    m_connection->close();
    m_openState.SetClosed();
  }
}

#endif
