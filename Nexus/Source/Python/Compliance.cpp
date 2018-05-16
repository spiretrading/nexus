#include "Nexus/Python/Compliance.hpp"
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/Copy.hpp>
#include <Beam/Python/Exception.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/PythonBindings.hpp>
#include <Beam/Python/Queues.hpp>
#include <Beam/Python/UniquePtr.hpp>
#include <Beam/Python/Variant.hpp>
#include <Beam/Python/Vector.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClientBuilder.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceClient.hpp"
#include "Nexus/Compliance/ComplianceParameter.hpp"
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"
#include "Nexus/Python/ToPythonComplianceClient.hpp"

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
using namespace boost::python;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace std;

namespace {
  using SessionBuilder = AuthenticatedServiceProtocolClientBuilder<
    VirtualServiceLocatorClient, MessageProtocol<
    std::unique_ptr<TcpSocketChannel>, BinarySender<SharedBuffer>, NullEncoder>,
    LiveTimer>;
  using Client = ComplianceClient<SessionBuilder>;

  struct FromPythonComplianceClient : VirtualComplianceClient,
      wrapper<VirtualComplianceClient> {

    virtual std::vector<ComplianceRuleEntry> Load(
        const DirectoryEntry& directoryEntry) override final {
      return get_override("load")(directoryEntry);
    }

    virtual ComplianceRuleId Add(const DirectoryEntry& directoryEntry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema)
        override final {
      return get_override("add")(directoryEntry, state, schema);
    }

    virtual void Update(const ComplianceRuleEntry& entry) override final {
      get_override("update")(entry);
    }

    virtual void Delete(ComplianceRuleId id) override final {
      get_override("delete")(id);
    }

    virtual void Report(const ComplianceRuleViolationRecord& violationRecord)
        override final {
      get_override("report")(violationRecord);
    }

    virtual void MonitorComplianceRuleEntries(
        const DirectoryEntry& directoryEntry,
        const std::shared_ptr<QueueWriter<ComplianceRuleEntry>>& queue,
        Out<vector<ComplianceRuleEntry>> snapshot) override final {
      get_override("monitor_compliance_rule_entries")(directoryEntry,
        queue, snapshot);
    }

    virtual void Open() override final {
      get_override("open")();
    }

    virtual void Close() override final {
      get_override("close")();
    }
  };

  auto BuildClient(VirtualServiceLocatorClient& serviceLocatorClient) {
    auto addresses = LocateServiceAddresses(serviceLocatorClient,
      Compliance::SERVICE_NAME);
    auto delay = false;
    SessionBuilder sessionBuilder(Ref(serviceLocatorClient),
      [=] () mutable {
        if(delay) {
          LiveTimer delayTimer(seconds(3), Ref(*GetTimerThreadPool()));
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
      sessionBuilder)).release();
  }
}

void Nexus::Python::ExportApplicationComplianceClient() {
  class_<ToPythonComplianceClient<Client>, bases<VirtualComplianceClient>,
    boost::noncopyable>("ApplicationComplianceClient", no_init)
    .def("__init__", make_constructor(&BuildClient));
}

void Nexus::Python::ExportComplianceClient() {
  class_<FromPythonComplianceClient, boost::noncopyable>("ComplianceClient",
    no_init)
    .def("load", pure_virtual(&VirtualComplianceClient::Load))
    .def("add", pure_virtual(&VirtualComplianceClient::Add))
    .def("update", pure_virtual(&VirtualComplianceClient::Update))
    .def("delete", pure_virtual(&VirtualComplianceClient::Delete))
    .def("report", pure_virtual(&VirtualComplianceClient::Report))
    .def("monitor_compliance_rule_entries",
      pure_virtual(&VirtualComplianceClient::MonitorComplianceRuleEntries))
    .def("open", pure_virtual(&VirtualComplianceClient::Open))
    .def("close", pure_virtual(&VirtualComplianceClient::Close));
  ExportUniquePtr<VirtualComplianceClient>();
}

void Nexus::Python::ExportCompliance() {
  string nestedName = extract<string>(scope().attr("__name__") + ".compliance");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("compliance") = nestedModule;
  scope parent = nestedModule;
  ExportComplianceClient();
  ExportApplicationComplianceClient();
  ExportComplianceParameter();
  ExportComplianceRuleEntry();
  ExportComplianceRuleSchema();
  ExportComplianceRuleViolationRecord();
  ExportException<ComplianceCheckException, std::runtime_error>(
    "ComplianceCheckException")
    .def(init<const string&>());
}

void Nexus::Python::ExportComplianceParameter() {
  ExportVariant<ComplianceValue>();
  ExportVector<vector<ComplianceValue>>("VectorComplianceValue");
  class_<ComplianceParameter>("ComplianceParameter", init<>())
    .def(init<std::string, ComplianceValue>())
    .def_readwrite("name", &ComplianceParameter::m_name)
    .add_property("value", make_getter(&ComplianceParameter::m_value,
      return_value_policy<return_by_value>()), make_setter(
      &ComplianceParameter::m_value, return_value_policy<return_by_value>()))
    .def(self == self)
    .def(self != self);
  ExportVector<vector<ComplianceParameter>>("VectorComplianceParameter");
}

void Nexus::Python::ExportComplianceRuleEntry() {
  class_<ComplianceRuleEntry>("ComplianceRuleEntry", init<>())
    .def(init<ComplianceRuleId, DirectoryEntry, ComplianceRuleEntry::State,
      ComplianceRuleSchema>())
    .add_property("id", &ComplianceRuleEntry::GetId,
      &ComplianceRuleEntry::SetId)
    .add_property("directory_entry", make_function(
      &ComplianceRuleEntry::GetDirectoryEntry,
      return_value_policy<copy_const_reference>()))
    .add_property("state", &ComplianceRuleEntry::GetState,
      &ComplianceRuleEntry::SetState)
    .add_property("schema", make_function(&ComplianceRuleEntry::GetSchema,
      return_value_policy<copy_const_reference>()))
    .def(self == self)
    .def(self != self);
  ExportVector<vector<ComplianceRuleEntry>>("VectorComplianceRuleEntry");
}

void Nexus::Python::ExportComplianceRuleSchema() {
  class_<ComplianceRuleSchema>("ComplianceRuleSchema", init<>())
    .def(init<string, vector<ComplianceParameter>>())
    .add_property("name", make_function(
      &ComplianceRuleSchema::GetName, return_value_policy<return_by_value>()))
    .add_property("parameters",
      make_function(&ComplianceRuleSchema::GetParameters,
      return_value_policy<return_by_value>()))
    .def(self == self)
    .def(self != self);
  ExportVector<vector<ComplianceRuleSchema>>("VectorComplianceRuleSchema");
}

void Nexus::Python::ExportComplianceRuleViolationRecord() {
  class_<ComplianceRuleViolationRecord>("ComplianceRuleViolationRecord",
    init<>())
    .def(init<DirectoryEntry, OrderId, ComplianceRuleId, string, string>())
    .def(init<DirectoryEntry, OrderId, ComplianceRuleId, string, string,
      ptime>())
    .add_property("account",
      make_getter(&ComplianceRuleViolationRecord::m_account,
      return_value_policy<return_by_value>()), make_setter(
      &ComplianceRuleViolationRecord::m_account,
      return_value_policy<return_by_value>()))
    .def_readwrite("order_id", &ComplianceRuleViolationRecord::m_orderId)
    .def_readwrite("rule_id", &ComplianceRuleViolationRecord::m_ruleId)
    .def_readwrite("schema_name", &ComplianceRuleViolationRecord::m_schemaName)
    .def_readwrite("reason", &ComplianceRuleViolationRecord::m_reason)
    .add_property("timestamp",
      make_getter(&ComplianceRuleViolationRecord::m_timestamp,
      return_value_policy<return_by_value>()), make_setter(
      &ComplianceRuleViolationRecord::m_timestamp,
      return_value_policy<return_by_value>()));
}
