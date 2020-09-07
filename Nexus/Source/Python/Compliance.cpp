#include "Nexus/Python/Compliance.hpp"
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Python/Beam.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClientBuilder.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceClient.hpp"
#include "Nexus/Compliance/ComplianceParameter.hpp"
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"
#include "Nexus/Python/ComplianceClient.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
  struct TrampolineComplianceClient final : VirtualComplianceClient {
    std::vector<ComplianceRuleEntry> Load(
        const DirectoryEntry& directoryEntry) override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<ComplianceRuleEntry>,
        VirtualComplianceClient, "load", Load, directoryEntry);
    }

    ComplianceRuleId Add(const DirectoryEntry& directoryEntry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema)
        override {
      PYBIND11_OVERLOAD_PURE_NAME(ComplianceRuleId, VirtualComplianceClient,
        "add", Add, directoryEntry, state, schema);
    }

    void Update(const ComplianceRuleEntry& entry) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualComplianceClient, "update",
        Update, entry);
    }

    void Delete(ComplianceRuleId id) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualComplianceClient, "delete",
        Delete, id);
    }

    void Report(const ComplianceRuleViolationRecord& violationRecord)
        override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualComplianceClient, "report",
        Report, violationRecord);
    }

    void MonitorComplianceRuleEntries(const DirectoryEntry& directoryEntry,
        const std::shared_ptr<QueueWriter<ComplianceRuleEntry>>& queue,
        Out<std::vector<ComplianceRuleEntry>> snapshot) override {
      throw std::runtime_error("Not implemented.");
    }

    void Close() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualComplianceClient, "close",
        Close);
    }
  };
}

void Nexus::Python::ExportApplicationComplianceClient(
    pybind11::module& module) {
  using SessionBuilder = AuthenticatedServiceProtocolClientBuilder<
    VirtualServiceLocatorClient, MessageProtocol<
    std::unique_ptr<TcpSocketChannel>, BinarySender<SharedBuffer>, NullEncoder>,
    LiveTimer>;
  using Client = ComplianceClient<SessionBuilder>;
  class_<ToPythonComplianceClient<Client>, VirtualComplianceClient>(module,
    "ApplicationComplianceClient")
    .def(init([] (VirtualServiceLocatorClient& serviceLocatorClient) {
      auto addresses = LocateServiceAddresses(serviceLocatorClient,
        Compliance::SERVICE_NAME);
      auto delay = false;
      auto sessionBuilder = SessionBuilder(Ref(serviceLocatorClient),
        [=] () mutable {
          if(delay) {
            auto delayTimer = LiveTimer(seconds(3), Ref(*GetTimerThreadPool()));
            delayTimer.Start();
            delayTimer.Wait();
          }
          delay = true;
          return std::make_unique<TcpSocketChannel>(addresses,
            Ref(*GetSocketThreadPool()));
        },
        [=] {
          return std::make_unique<LiveTimer>(seconds(10),
            Ref(*GetTimerThreadPool()));
        });
      return MakeToPythonComplianceClient(std::make_unique<Client>(
        sessionBuilder));
    }), call_guard<GilRelease>());
}

void Nexus::Python::ExportComplianceClient(pybind11::module& module) {
  class_<VirtualComplianceClient, TrampolineComplianceClient>(module,
      "ComplianceClient")
    .def("load", &VirtualComplianceClient::Load)
    .def("add", &VirtualComplianceClient::Add)
    .def("update", &VirtualComplianceClient::Update)
    .def("delete", &VirtualComplianceClient::Delete)
    .def("report", &VirtualComplianceClient::Report)
    .def("close", &VirtualComplianceClient::Close);
}

void Nexus::Python::ExportCompliance(pybind11::module& module) {
  auto submodule = module.def_submodule("compliance");
  ExportComplianceClient(submodule);
  ExportApplicationComplianceClient(submodule);
  ExportComplianceParameter(submodule);
  ExportComplianceRuleEntry(submodule);
  ExportComplianceRuleSchema(submodule);
  ExportComplianceRuleViolationRecord(submodule);
  register_exception<ComplianceCheckException>(submodule,
    "ComplianceCheckException");
}

void Nexus::Python::ExportComplianceParameter(pybind11::module& module) {
  class_<ComplianceParameter>(module, "ComplianceParameter")
    .def(init())
    .def(init<std::string, ComplianceValue>())
    .def_readwrite("name", &ComplianceParameter::m_name)
    .def_readwrite("value", &ComplianceParameter::m_value)
    .def(self == self)
    .def(self != self);
}

void Nexus::Python::ExportComplianceRuleEntry(pybind11::module& module) {
  class_<ComplianceRuleEntry>(module, "ComplianceRuleEntry")
    .def(init())
    .def(init<ComplianceRuleId, DirectoryEntry, ComplianceRuleEntry::State,
      ComplianceRuleSchema>())
    .def_property("id", &ComplianceRuleEntry::GetId,
      &ComplianceRuleEntry::SetId)
    .def_property_readonly("directory_entry",
      &ComplianceRuleEntry::GetDirectoryEntry)
    .def_property("state", &ComplianceRuleEntry::GetState,
      &ComplianceRuleEntry::SetState)
    .def_property_readonly("schema", &ComplianceRuleEntry::GetSchema)
    .def(self == self)
    .def(self != self);
}

void Nexus::Python::ExportComplianceRuleSchema(pybind11::module& module) {
  class_<ComplianceRuleSchema>(module, "ComplianceRuleSchema")
    .def(init())
    .def(init<std::string, std::vector<ComplianceParameter>>())
    .def_property_readonly("name", &ComplianceRuleSchema::GetName)
    .def_property_readonly("parameters", &ComplianceRuleSchema::GetParameters)
    .def(self == self)
    .def(self != self);
}

void Nexus::Python::ExportComplianceRuleViolationRecord(
    pybind11::module& module) {
  class_<ComplianceRuleViolationRecord>(module, "ComplianceRuleViolationRecord")
    .def(init())
    .def(init<DirectoryEntry, OrderId, ComplianceRuleId, std::string,
      std::string>())
    .def(init<DirectoryEntry, OrderId, ComplianceRuleId, std::string,
      std::string, ptime>())
    .def_readwrite("account", &ComplianceRuleViolationRecord::m_account)
    .def_readwrite("order_id", &ComplianceRuleViolationRecord::m_orderId)
    .def_readwrite("rule_id", &ComplianceRuleViolationRecord::m_ruleId)
    .def_readwrite("schema_name", &ComplianceRuleViolationRecord::m_schemaName)
    .def_readwrite("reason", &ComplianceRuleViolationRecord::m_reason)
    .def_readwrite("timestamp", &ComplianceRuleViolationRecord::m_timestamp);
}
