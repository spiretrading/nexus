#ifndef NEXUS_LOCALADMINISTRATIONDATASTORE_HPP
#define NEXUS_LOCALADMINISTRATIONDATASTORE_HPP
#include <unordered_map>
#include <Beam/IO/OpenState.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/thread/mutex.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

namespace Nexus {
namespace AdministrationService {

  /*! \class LocalAdministrationDataStore
      \brief Stores Nexus account info locally.
   */
  class LocalAdministrationDataStore : public AdministrationDataStore {
    public:

      //! Constructs an empty LocalAdministrationDataStore.
      LocalAdministrationDataStore();

      virtual ~LocalAdministrationDataStore();

      virtual AccountIdentity LoadIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account);

      virtual void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity);

      virtual RiskService::RiskParameters LoadRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account);

      virtual void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters);

      virtual RiskService::RiskState LoadRiskState(
        const Beam::ServiceLocator::DirectoryEntry& account);

      virtual void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& riskState);

      virtual void WithTransaction(const std::function<void ()>& transaction);

      virtual void Open();

      virtual void Close();

    private:
      mutable boost::mutex m_mutex;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry, AccountIdentity>
        m_identities;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
        RiskService::RiskParameters> m_riskParameters;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
        RiskService::RiskState> m_riskStates;
      Beam::IO::OpenState m_openState;
  };

  inline LocalAdministrationDataStore::LocalAdministrationDataStore() {}

  inline LocalAdministrationDataStore::~LocalAdministrationDataStore() {
    Close();
  }

  inline AccountIdentity LocalAdministrationDataStore::LoadIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_identities[account];
  }

  inline void LocalAdministrationDataStore::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    m_identities[account] = identity;
  }

  inline RiskService::RiskParameters LocalAdministrationDataStore::
      LoadRiskParameters(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_riskParameters[account];
  }

  inline void LocalAdministrationDataStore::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    m_riskParameters[account] = riskParameters;
  }

  inline RiskService::RiskState LocalAdministrationDataStore::LoadRiskState(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_riskStates[account];
  }

  inline void LocalAdministrationDataStore::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    m_riskStates[account] = riskState;
  }

  inline void LocalAdministrationDataStore::WithTransaction(
      const std::function<void ()>& transaction) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    transaction();
  }

  inline void LocalAdministrationDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_openState.SetOpen();
  }

  inline void LocalAdministrationDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_openState.SetClosed();
  }
}
}

#endif
