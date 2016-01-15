#ifndef NEXUS_VIRTUALCOMPLIANCECLIENT_HPP
#define NEXUS_VIRTUALCOMPLIANCECLIENT_HPP
#include <memory>
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Queues/QueueWriter.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class VirtualComplianceClient
      \brief Provides a pure virtual interface to a ComplianceClient.
   */
  class VirtualComplianceClient : private boost::noncopyable {
    public:
      virtual ~VirtualComplianceClient() = default;

      virtual std::vector<ComplianceRuleEntry> Load(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry) = 0;

      virtual ComplianceRuleId Add(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
        ComplianceRuleEntry::State state,
        const ComplianceRuleSchema& schema) = 0;

      virtual void Update(const ComplianceRuleEntry& entry) = 0;

      virtual void Delete(ComplianceRuleId id) = 0;

      virtual void Report(
        const ComplianceRuleViolationRecord& violationRecord) = 0;

      virtual void MonitorComplianceRuleEntries(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
        const std::shared_ptr<Beam::QueueWriter<ComplianceRuleEntry>>& queue,
        Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) = 0;

      virtual void Open() = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualComplianceClient.
      VirtualComplianceClient() = default;
  };

  /*! \class WrapperComplianceClient
      \brief Wraps a ComplianceClient providing it with a virtual interface.
      \tparam ClientType The type of ComplianceClient to wrap.
   */
  template<typename ClientType>
  class WrapperComplianceClient : public VirtualComplianceClient {
    public:

      //! The ComplianceClient to wrap.
      using Client = Beam::GetTryDereferenceType<ClientType>;

      //! Constructs a WrapperComplianceClient.
      /*!
        \param client The ComplianceClient to wrap.
      */
      template<typename ComplianceClientForward>
      WrapperComplianceClient(ComplianceClientForward&& client);

      virtual std::vector<ComplianceRuleEntry> Load(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry);

      virtual ComplianceRuleId Add(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema);

      virtual void Update(const ComplianceRuleEntry& entry);

      virtual void Delete(ComplianceRuleId id);

      virtual void Report(const ComplianceRuleViolationRecord& violationRecord);

      virtual void MonitorComplianceRuleEntries(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
        const std::shared_ptr<Beam::QueueWriter<ComplianceRuleEntry>>& queue,
        Beam::Out<std::vector<ComplianceRuleEntry>> snapshot);

      virtual void Open();

      virtual void Close();

    private:
      typename Beam::OptionalLocalPtr<ClientType>::type m_client;
  };

  //! Wraps a ComplianceClient into a VirtualComplianceClient.
  /*!
    \param client The client to wrap.
  */
  template<typename ComplianceClient>
  std::unique_ptr<VirtualComplianceClient> MakeVirtualComplianceClient(
      ComplianceClient&& client) {
    return std::make_unique<WrapperComplianceClient<ComplianceClient>>(
      std::forward<ComplianceClient>(client));
  }

  template<typename ClientType>
  template<typename ComplianceClientForward>
  WrapperComplianceClient<ClientType>::WrapperComplianceClient(
      ComplianceClientForward&& client)
      : m_client{std::forward<ComplianceClientForward>(client)} {}

  template<typename ClientType>
  std::vector<ComplianceRuleEntry> WrapperComplianceClient<ClientType>::Load(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    return m_client->Load(directoryEntry);
  }

  template<typename ClientType>
  ComplianceRuleId WrapperComplianceClient<ClientType>::Add(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    return m_client->Add(directoryEntry, state, schema);
  }

  template<typename ClientType>
  void WrapperComplianceClient<ClientType>::Update(
      const ComplianceRuleEntry& entry) {
    return m_client->Update(entry);
  }

  template<typename ClientType>
  void WrapperComplianceClient<ClientType>::Delete(ComplianceRuleId id) {
    return m_client->Delete(id);
  }

  template<typename ClientType>
  void WrapperComplianceClient<ClientType>::Report(
      const ComplianceRuleViolationRecord& violationRecord) {
    return m_client->Report(violationRecord);
  }

  template<typename ClientType>
  void WrapperComplianceClient<ClientType>::MonitorComplianceRuleEntries(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      const std::shared_ptr<Beam::QueueWriter<ComplianceRuleEntry>>& queue,
      Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    return m_client->MonitorComplianceRuleEntries(directoryEntry, queue,
      Beam::Store(snapshot));
  }

  template<typename ClientType>
  void WrapperComplianceClient<ClientType>::Open() {
    return m_client->Open();
  }

  template<typename ClientType>
  void WrapperComplianceClient<ClientType>::Close() {
    return m_client->Close();
  }
}
}

#endif
