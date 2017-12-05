#include "Nexus/Python/AdministrationService.hpp"
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/Copy.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/PythonBindings.hpp>
#include <Beam/Python/Queues.hpp>
#include <Beam/Python/UniquePtr.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClientBuilder.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/AdministrationService/EntitlementModification.hpp"
#include "Nexus/AdministrationService/Message.hpp"
#include "Nexus/AdministrationService/TradingGroup.hpp"
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Python/ToPythonAdministrationClient.hpp"

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
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::MarketDataService;
using namespace Nexus::Python;
using namespace Nexus::RiskService;
using namespace std;

namespace {
  using SessionBuilder = AuthenticatedServiceProtocolClientBuilder<
    VirtualServiceLocatorClient, MessageProtocol<
    std::unique_ptr<TcpSocketChannel>, BinarySender<SharedBuffer>, NullEncoder>,
    LiveTimer>;
  using Client = AdministrationClient<SessionBuilder>;

  struct FromPythonAdministrationClient : VirtualAdministrationClient,
      wrapper<VirtualAdministrationClient> {
    virtual bool CheckAdministrator(
        const DirectoryEntry& account) override final {
      return get_override("check_administrator")(account);
    }

    virtual AccountRoles LoadAccountRoles(
        const DirectoryEntry& account) override final {
      return get_override("load_account_roles")(account);
    }

    virtual AccountRoles LoadAccountRoles(const DirectoryEntry& parent,
        const DirectoryEntry& child) override final {
      return get_override("load_account_roles")(parent, child);
    }

    virtual DirectoryEntry LoadTradingGroupEntry(
        const DirectoryEntry& account) override final {
      return get_override("load_trading_group_entry")(account);
    }

    virtual AccountIdentity LoadIdentity(
        const DirectoryEntry& account) override final {
      return get_override("load_identity")(account);
    }

    virtual void StoreIdentity(const DirectoryEntry& account,
        const AccountIdentity& identity) override final {
      get_override("store_identity")(account, identity);
    }

    virtual TradingGroup LoadTradingGroup(
        const DirectoryEntry& directory) override final {
      return get_override("load_trading_group")(directory);
    }

    virtual vector<DirectoryEntry> LoadManagedTradingGroups(
        const DirectoryEntry& account) override final {
      return get_override("load_managed_trading_groups")(account);
    }

    virtual vector<DirectoryEntry> LoadAdministrators() override final {
      return get_override("load_administrators")();
    }

    virtual vector<DirectoryEntry> LoadServices() override final {
      return get_override("load_services")();
    }

    virtual EntitlementDatabase LoadEntitlements() override final {
      return get_override("load_entitlements")();
    }

    virtual vector<DirectoryEntry> LoadEntitlements(
        const DirectoryEntry& account) override final {
      return get_override("load_entitlements")(account);
    }

    virtual void StoreEntitlements(const DirectoryEntry& account,
        const vector<DirectoryEntry>& entitlements) override final {
      get_override("store_entitlements")(account, entitlements);
    }

    virtual const Publisher<RiskParameters>& GetRiskParametersPublisher(
        const DirectoryEntry& account) override final {
      return *static_cast<const Publisher<RiskParameters>*>(
        get_override("get_risk_parameters_publisher")(account));
    }

    virtual void StoreRiskParameters(const DirectoryEntry& account,
        const RiskParameters& riskParameters) override final {
      get_override("store_risk_parameters")(account, riskParameters);
    }

    virtual const Publisher<RiskState>& GetRiskStatePublisher(
        const DirectoryEntry& account) override final {
      return *static_cast<const Publisher<RiskState>*>(
        get_override("get_risk_state_publisher")(account));
    }

    virtual void StoreRiskState(const DirectoryEntry& account,
        const RiskState& riskState) override final {
      get_override("store_risk_state")(account, riskState);
    }

    virtual AccountModificationRequest LoadAccountModificationRequest(
        AccountModificationRequest::Id id) override final {
      return get_override("load_account_modification_request")(id);
    }

    virtual vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(const DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) override final {
      return get_override("load_account_modification_request_ids")(account,
        startId, maxCount);
    }

    virtual vector<AccountModificationRequest::Id>
        LoadManagedAccountModificationRequestIds(const DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) override final {
      return get_override("load_managed_account_modification_request_ids")(
        account, startId, maxCount);
    }

    virtual EntitlementModification LoadEntitlementModification(
        AccountModificationRequest::Id id) override final {
      return get_override("load_entitlement_modification")(id);
    }

    virtual AccountModificationRequest SubmitAccountModificationRequest(
        const DirectoryEntry& account, const DirectoryEntry& submissionAccount,
        const EntitlementModification& modification,
        const AdministrationService::Message& comment) override final {
      return get_override("submit_account_modification_request")(account,
        submissionAccount, modification, comment);
    }

    virtual AccountModificationRequest::Update
        LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id) override final {
      return get_override("load_account_modification_request_status")(id);
    }

    virtual AccountModificationRequest::Update
        ApproveAccountModificationRequest(AccountModificationRequest::Id id,
        const DirectoryEntry& account,
        const AdministrationService::Message& comment) override final {
      return get_override("approve_account_modification_request")(id, account,
        comment);
    }

    virtual AccountModificationRequest::Update RejectAccountModificationRequest(
        AccountModificationRequest::Id id, const DirectoryEntry& account,
        const AdministrationService::Message& comment) override final {
      return get_override("reject_account_modification_request")(id, account,
        comment);
    }

    virtual AdministrationService::Message LoadMessage(
        AdministrationService::Message::Id id) override final {
      return get_override("load_message")(id);
    }

    virtual vector<AdministrationService::Message::Id> LoadMessageIds(
        AccountModificationRequest::Id id) override final {
      return get_override("load_message_ids")(id);
    }

    virtual AdministrationService::Message
        SendAccountModificationRequestMessage(AccountModificationRequest::Id id,
        const AdministrationService::Message& message) override final {
      return get_override("send_account_modification_request_message")(id,
        message);
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
      AdministrationService::SERVICE_NAME);
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
    return MakeToPythonAdministrationClient(std::make_unique<Client>(
      sessionBuilder)).release();
  }

  auto BuildAdministrationServiceTestEnvironment(
      const std::shared_ptr<VirtualServiceLocatorClient>&
      serviceLocatorClient) {
    return new AdministrationServiceTestEnvironment{serviceLocatorClient};
  }

  auto AdministrationServiceTestEnvironmentBuildClient(
      AdministrationServiceTestEnvironment& environment,
      VirtualServiceLocatorClient& serviceLocatorClient) {
    return MakeToPythonAdministrationClient(environment.BuildClient(
      Ref(serviceLocatorClient)));
  }
}

BEAM_DEFINE_PYTHON_QUEUE_LINKER(RiskParameters);
BEAM_DEFINE_PYTHON_QUEUE_LINKER(RiskState);
BEAM_DEFINE_PYTHON_POINTER_LINKER(VirtualAdministrationClient);

void Nexus::Python::ExportAccountIdentity() {
  class_<AccountIdentity>("AccountIdentity", init<>())
    .def("__copy__", &MakeCopy<AccountIdentity>)
    .def("__deepcopy__", &MakeDeepCopy<AccountIdentity>)
    .add_property("registration_time", make_getter(
      &AccountIdentity::m_registrationTime,
      return_value_policy<return_by_value>()), make_setter(
      &AccountIdentity::m_registrationTime,
      return_value_policy<return_by_value>()))
    .add_property("last_login_time", make_getter(
      &AccountIdentity::m_lastLoginTime,
      return_value_policy<return_by_value>()), make_setter(
      &AccountIdentity::m_lastLoginTime,
      return_value_policy<return_by_value>()))
    .def_readwrite("first_name", &AccountIdentity::m_firstName)
    .def_readwrite("last_name", &AccountIdentity::m_lastName)
    .def_readwrite("email_address", &AccountIdentity::m_emailAddress)
    .def_readwrite("address_line_one", &AccountIdentity::m_addressLineOne)
    .def_readwrite("address_line_two", &AccountIdentity::m_addressLineTwo)
    .def_readwrite("address_line_three", &AccountIdentity::m_addressLineThree)
    .def_readwrite("city", &AccountIdentity::m_city)
    .def_readwrite("province", &AccountIdentity::m_province)
    .def_readwrite("country", &AccountIdentity::m_country)
    .def_readwrite("photo_id", &AccountIdentity::m_photoId)
    .def_readwrite("user_notes", &AccountIdentity::m_userNotes);
}

void Nexus::Python::ExportAccountModificationRequest() {
  {
    scope outer =
      class_<AccountModificationRequest>("AccountModificationRequest",
        init<AccountModificationRequest::Id, AccountModificationRequest::Type,
        DirectoryEntry, DirectoryEntry, ptime>())
        .def("__copy__", &MakeCopy<AccountModificationRequest>)
        .def("__deepcopy__", &MakeDeepCopy<AccountModificationRequest>)
        .add_property("id", &AccountModificationRequest::GetId)
        .add_property("type", &AccountModificationRequest::GetType)
        .add_property("account", make_function(
          &AccountModificationRequest::GetAccount,
          return_value_policy<return_by_value>()))
        .add_property("submission_account", make_function(
          &AccountModificationRequest::GetSubmissionAccount,
          return_value_policy<return_by_value>()))
        .add_property("timestamp", make_function(
          &AccountModificationRequest::GetTimestamp,
          return_value_policy<return_by_value>()));
    enum_<AccountModificationRequest::Type>("Type")
      .value("ENTITLEMENTS", AccountModificationRequest::Type::ENTITLEMENTS);
    enum_<AccountModificationRequest::Status>("Status")
      .value("NONE", AccountModificationRequest::Status::NONE)
      .value("PENDING", AccountModificationRequest::Status::PENDING)
      .value("REVIEWED", AccountModificationRequest::Status::REVIEWED)
      .value("SCHEDULED", AccountModificationRequest::Status::SCHEDULED)
      .value("GRANTED", AccountModificationRequest::Status::GRANTED)
      .value("REJECTED", AccountModificationRequest::Status::REJECTED);
    class_<AccountModificationRequest::Update>("Update", init<>())
      .def(init<AccountModificationRequest::Status, DirectoryEntry, int,
        boost::posix_time::ptime>())
      .def_readwrite("status", &AccountModificationRequest::Update::m_status)
      .def_readwrite("account", &AccountModificationRequest::Update::m_account)
      .def_readwrite("sequence_number",
        &AccountModificationRequest::Update::m_sequenceNumber)
      .add_property("timestamp", make_getter(
        &AccountModificationRequest::Update::m_timestamp,
        return_value_policy<return_by_value>()), make_setter(
        &AccountModificationRequest::Update::m_timestamp,
        return_value_policy<return_by_value>()));
  }
}

void Nexus::Python::ExportAdministrationClient() {
  class_<FromPythonAdministrationClient, boost::noncopyable>(
    "AdministrationClient", no_init)
    .def("check_administrator",
      pure_virtual(&VirtualAdministrationClient::CheckAdministrator))
    .def("load_account_roles", pure_virtual(
      static_cast<AccountRoles (VirtualAdministrationClient::*)(
      const DirectoryEntry&)>(&VirtualAdministrationClient::LoadAccountRoles)))
    .def("load_account_roles", pure_virtual(
      static_cast<AccountRoles (VirtualAdministrationClient::*)(
      const DirectoryEntry&, const DirectoryEntry&)>(
      &VirtualAdministrationClient::LoadAccountRoles)))
    .def("load_trading_group_entry",
      pure_virtual(&VirtualAdministrationClient::LoadTradingGroupEntry))
    .def("load_identity",
      pure_virtual(&VirtualAdministrationClient::LoadIdentity))
    .def("store_identity",
      pure_virtual(&VirtualAdministrationClient::StoreIdentity))
    .def("load_trading_group",
      pure_virtual(&VirtualAdministrationClient::LoadTradingGroup))
    .def("load_managed_trading_groups",
      pure_virtual(&VirtualAdministrationClient::LoadManagedTradingGroups))
    .def("load_administrators",
      pure_virtual(&VirtualAdministrationClient::LoadAdministrators))
    .def("load_services",
      pure_virtual(&VirtualAdministrationClient::LoadServices))
    .def("load_entitlements", pure_virtual(
      static_cast<EntitlementDatabase (VirtualAdministrationClient::*)()>(
      &VirtualAdministrationClient::LoadEntitlements)))
    .def("load_entitlements", pure_virtual(
      static_cast<vector<DirectoryEntry> (VirtualAdministrationClient::*)(
      const DirectoryEntry&)>(&VirtualAdministrationClient::LoadEntitlements)))
    .def("store_entitlements",
      pure_virtual(&VirtualAdministrationClient::StoreEntitlements))
    .def("get_risk_parameters_publisher",
      pure_virtual(&VirtualAdministrationClient::GetRiskParametersPublisher),
      return_internal_reference<>())
    .def("store_risk_parameters",
      pure_virtual(&VirtualAdministrationClient::StoreRiskParameters))
    .def("get_risk_state_publisher",
      pure_virtual(&VirtualAdministrationClient::GetRiskStatePublisher),
      return_internal_reference<>())
    .def("store_risk_state",
      pure_virtual(&VirtualAdministrationClient::StoreRiskState))
    .def("load_account_modification_request", pure_virtual(
      &VirtualAdministrationClient::LoadAccountModificationRequest))
    .def("load_account_modification_request_ids", pure_virtual(
      &VirtualAdministrationClient::LoadAccountModificationRequestIds))
    .def("load_managed_account_modification_request_ids", pure_virtual(
      &VirtualAdministrationClient::LoadManagedAccountModificationRequestIds))
    .def("load_entitlement_modification", pure_virtual(
      &VirtualAdministrationClient::LoadEntitlementModification))
    .def("submit_account_modification_request", pure_virtual(
      &VirtualAdministrationClient::SubmitAccountModificationRequest))
    .def("load_account_modification_request_status", pure_virtual(
      &VirtualAdministrationClient::LoadAccountModificationRequestStatus))
    .def("approve_account_modification_request", pure_virtual(
      &VirtualAdministrationClient::ApproveAccountModificationRequest))
    .def("reject_account_modification_request", pure_virtual(
      &VirtualAdministrationClient::RejectAccountModificationRequest))
    .def("load_message", pure_virtual(
      &VirtualAdministrationClient::LoadMessage))
    .def("load_message_ids", pure_virtual(
      &VirtualAdministrationClient::LoadMessageIds))
    .def("send_account_modification_request_message", pure_virtual(
      &VirtualAdministrationClient::SendAccountModificationRequestMessage))
    .def("open", pure_virtual(&VirtualAdministrationClient::Open))
    .def("close", pure_virtual(&VirtualAdministrationClient::Close));
  ExportUniquePtr<VirtualAdministrationClient>();
}

void Nexus::Python::ExportAdministrationService() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".administration_service");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("administration_service") = nestedModule;
  scope parent = nestedModule;
  ExportAccountIdentity();
  ExportAccountModificationRequest();
  ExportAdministrationClient();
  ExportApplicationAdministrationClient();
  ExportEntitlementModification();
  ExportMessage();
  ExportTradingGroup();
  ExportQueueSuite<RiskState>("RiskState");
  ExportQueueSuite<RiskParameters>("RiskParameters");
  {
    string nestedName = extract<string>(parent.attr("__name__") + ".tests");
    object nestedModule{handle<>(
      borrowed(PyImport_AddModule(nestedName.c_str())))};
    parent.attr("tests") = nestedModule;
    scope child = nestedModule;
    ExportAdministrationServiceTestEnvironment();
  }
}

void Nexus::Python::ExportAdministrationServiceTestEnvironment() {
  class_<AdministrationServiceTestEnvironment, boost::noncopyable>(
      "AdministrationServiceTestEnvironment", no_init)
    .def("__init__",
      make_constructor(BuildAdministrationServiceTestEnvironment))
    .def("open", BlockingFunction(&AdministrationServiceTestEnvironment::Open))
    .def("close", BlockingFunction(
      &AdministrationServiceTestEnvironment::Close))
    .def("make_administrator", BlockingFunction(
      &AdministrationServiceTestEnvironment::MakeAdministrator))
    .def("build_client", &AdministrationServiceTestEnvironmentBuildClient);
}

void Nexus::Python::ExportApplicationAdministrationClient() {
  class_<ToPythonAdministrationClient<Client>,
    bases<VirtualAdministrationClient>, boost::noncopyable>(
    "ApplicationAdministrationClient", no_init)
    .def("__init__", make_constructor(&BuildClient));
}

void Nexus::Python::ExportEntitlementModification() {
  class_<EntitlementModification>("EntitlementModification", init<>())
    .def(init<vector<DirectoryEntry>>())
    .def("__copy__", &MakeCopy<EntitlementModification>)
    .def("__deepcopy__", &MakeDeepCopy<EntitlementModification>)
    .add_property("entitlements", make_function(
      &EntitlementModification::GetEntitlements,
      return_value_policy<return_by_value>()));
}

void Nexus::Python::ExportMessage() {
  {
    scope outer =
      class_<AdministrationService::Message>("Message", init<>())
        .def(init<AdministrationService::Message::Id, DirectoryEntry, ptime,
        vector<AdministrationService::Message::Body>>())
        .def("__copy__", &MakeCopy<AdministrationService::Message>)
        .def("__deepcopy__", &MakeDeepCopy<AdministrationService::Message>)
        .add_property("id", &AdministrationService::Message::GetId)
        .add_property("account", make_function(
          &AdministrationService::Message::GetAccount,
          return_value_policy<return_by_value>()))
        .add_property("timestamp", make_function(
          &AdministrationService::Message::GetTimestamp,
          return_value_policy<return_by_value>()))
        .add_property("body", make_function(
          &AdministrationService::Message::GetBody,
          return_value_policy<return_by_value>()))
        .add_property("bodies", make_function(
          &AdministrationService::Message::GetBodies,
          return_value_policy<return_by_value>()));
      class_<AdministrationService::Message::Body>("Body")
        .def("__copy__", &MakeCopy<AdministrationService::Message::Body>)
        .def("__deepcopy__", &MakeDeepCopy<
          AdministrationService::Message::Body>)
        .def_readwrite("content_type",
          &AdministrationService::Message::Body::m_contentType)
        .def_readwrite("message",
          &AdministrationService::Message::Body::m_message);
  }
}

void Nexus::Python::ExportTradingGroup() {
  class_<TradingGroup>("TradingGroup", init<>())
    .def(init<const DirectoryEntry&, const DirectoryEntry&,
      const vector<DirectoryEntry>&, const DirectoryEntry&,
      const vector<DirectoryEntry>&>())
    .def("__copy__", &MakeCopy<TradingGroup>)
    .def("__deepcopy__", &MakeDeepCopy<TradingGroup>)
    .add_property("entry", make_function(&TradingGroup::GetEntry,
      return_value_policy<return_by_value>()))
    .add_property("managers", make_function(&TradingGroup::GetManagers,
      return_value_policy<return_by_value>()))
    .add_property("traders", make_function(&TradingGroup::GetTraders,
      return_value_policy<return_by_value>()));
}
