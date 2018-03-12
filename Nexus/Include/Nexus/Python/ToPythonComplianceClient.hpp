#ifndef NEXUS_TO_PYTHON_COMPLIANCE_CLIENT_HPP
#define NEXUS_TO_PYTHON_COMPLIANCE_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Python/GilRelease.hpp>
#include "Nexus/Compliance/VirtualComplianceClient.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class ToPythonComplianceClient
      \brief Wraps a ComplianceClient for use with Python.
      \tparam ClientType The type of ComplianceClient to wrap.
   */
  template<typename ClientType>
  class ToPythonComplianceClient : public VirtualComplianceClient {
    public:

      //! The type of ComplianceClient to wrap.
      using Client = ClientType;

      //! Constructs a ToPythonComplianceClient.
      /*!
        \param client The ComplianceClient to wrap.
      */
      ToPythonComplianceClient(std::unique_ptr<Client> client);

      virtual ~ToPythonComplianceClient() override final;

      virtual std::vector<ComplianceRuleEntry> Load(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry)
        override final;

      virtual ComplianceRuleId Add(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema)
        override final;

      virtual void Update(const ComplianceRuleEntry& entry) override final;

      virtual void Delete(ComplianceRuleId id) override final;

      virtual void Report(const ComplianceRuleViolationRecord& violationRecord)
        override final;

      virtual void MonitorComplianceRuleEntries(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
        const std::shared_ptr<Beam::QueueWriter<ComplianceRuleEntry>>& queue,
        Beam::Out<std::vector<ComplianceRuleEntry>> snapshot)
        override final;

      virtual void Open() override final;

      virtual void Close() override final;

    private:
      std::unique_ptr<Client> m_client;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  //! Makes a ToPythonComplianceClient.
  /*!
    \param client The ComplianceClient to wrap.
  */
  template<typename Client>
  auto MakeToPythonComplianceClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonComplianceClient<Client>>(
      std::move(client));
  }

  template<typename ClientType>
  ToPythonComplianceClient<ClientType>::ToPythonComplianceClient(
      std::unique_ptr<Client> client)
      : m_client{std::move(client)} {}

  template<typename ClientType>
  ToPythonComplianceClient<ClientType>::~ToPythonComplianceClient() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    Close();
    m_client.reset();
  }

  template<typename ClientType>
  std::vector<ComplianceRuleEntry> ToPythonComplianceClient<ClientType>::Load(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->Load(directoryEntry);
  }

  template<typename ClientType>
  ComplianceRuleId ToPythonComplianceClient<ClientType>::Add(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->Add(directoryEntry, state, schema);
  }

  template<typename ClientType>
  void ToPythonComplianceClient<ClientType>::Update(
      const ComplianceRuleEntry& entry) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->Update(entry);
  }

  template<typename ClientType>
  void ToPythonComplianceClient<ClientType>::Delete(ComplianceRuleId id) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->Delete(id);
  }

  template<typename ClientType>
  void ToPythonComplianceClient<ClientType>::Report(
      const ComplianceRuleViolationRecord& violationRecord) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->Report(violationRecord);
  }

  template<typename ClientType>
  void ToPythonComplianceClient<ClientType>::MonitorComplianceRuleEntries(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      const std::shared_ptr<Beam::QueueWriter<ComplianceRuleEntry>>& queue,
      Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->MonitorComplianceRuleEntries(directoryEntry, queue,
      snapshot);
  }

  template<typename ClientType>
  void ToPythonComplianceClient<ClientType>::Open() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_client->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ClientType>
  void ToPythonComplianceClient<ClientType>::Close() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ClientType>
  void ToPythonComplianceClient<ClientType>::Shutdown() {
    m_client->Close();
    m_openState.SetClosed();
  }
}
}

#endif
