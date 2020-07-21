#include "Nexus/Python/AdministrationService.hpp"
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Python/Beam.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClientBuilder.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/Python/AdministrationClient.hpp"

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
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::MarketDataService;
using namespace Nexus::Python;
using namespace Nexus::RiskService;
using namespace pybind11;

namespace {
  struct TrampoilineAdministrationClient final : VirtualAdministrationClient {
    using VirtualAdministrationClient::VirtualAdministrationClient;

    std::vector<DirectoryEntry> LoadAccountsByRoles(
        AccountRoles roles) override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<DirectoryEntry>,
        VirtualAdministrationClient, "load_accounts_by_roles",
        LoadAccountsByRoles, roles);
    }

    DirectoryEntry LoadAdministratorsRootEntry() override {
      PYBIND11_OVERLOAD_PURE_NAME(DirectoryEntry, VirtualAdministrationClient,
        "load_administrators_root_entry", LoadAdministratorsRootEntry);
    }

    DirectoryEntry LoadServicesRootEntry() override {
      PYBIND11_OVERLOAD_PURE_NAME(DirectoryEntry, VirtualAdministrationClient,
        "load_services_root_entry", LoadServicesRootEntry);
    }

    DirectoryEntry LoadTradingGroupsRootEntry() override {
      PYBIND11_OVERLOAD_PURE_NAME(DirectoryEntry, VirtualAdministrationClient,
        "load_trading_groups_root_entry", LoadTradingGroupsRootEntry);
    }

    bool CheckAdministrator(const DirectoryEntry& account) override {
      PYBIND11_OVERLOAD_PURE_NAME(bool, VirtualAdministrationClient,
        "check_administrator", CheckAdministrator, account);
    }

    AccountRoles LoadAccountRoles(const DirectoryEntry& account) override {
      PYBIND11_OVERLOAD_PURE_NAME(AccountRoles, VirtualAdministrationClient,
        "load_account_roles", LoadAccountRoles, account);
    }

    AccountRoles LoadAccountRoles(const DirectoryEntry& parent,
        const DirectoryEntry& child) override {
      PYBIND11_OVERLOAD_PURE_NAME(AccountRoles, VirtualAdministrationClient,
        "load_account_roles", LoadAccountRoles, parent, child);
    }

    DirectoryEntry LoadTradingGroupEntry(
        const DirectoryEntry& account) override {
      PYBIND11_OVERLOAD_PURE_NAME(DirectoryEntry, VirtualAdministrationClient,
        "load_trading_group_entry", LoadTradingGroupEntry, account);
    }

    AccountIdentity LoadIdentity(const DirectoryEntry& account) override {
      PYBIND11_OVERLOAD_PURE_NAME(AccountIdentity, VirtualAdministrationClient,
        "load_identity", LoadIdentity, account);
    }

    void StoreIdentity(const DirectoryEntry& account,
        const AccountIdentity& identity) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualAdministrationClient,
        "store_identity", StoreIdentity, account, identity);
    }

    TradingGroup LoadTradingGroup(const DirectoryEntry& directory) override {
      PYBIND11_OVERLOAD_PURE_NAME(TradingGroup, VirtualAdministrationClient,
        "load_trading_group", LoadTradingGroup, directory);
    }

    std::vector<DirectoryEntry> LoadManagedTradingGroups(
        const DirectoryEntry& account) override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<DirectoryEntry>,
        VirtualAdministrationClient, "load_managed_trading_groups",
        LoadManagedTradingGroups, account);
    }

    std::vector<DirectoryEntry> LoadAdministrators() override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<DirectoryEntry>,
        VirtualAdministrationClient, "load_administrators", LoadAdministrators);
    }

    std::vector<DirectoryEntry> LoadServices() override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<DirectoryEntry>,
        VirtualAdministrationClient, "load_services", LoadServices);
    }

    EntitlementDatabase LoadEntitlements() override {
      PYBIND11_OVERLOAD_PURE_NAME(EntitlementDatabase,
        VirtualAdministrationClient, "load_entitlements", LoadEntitlements);
    }

    std::vector<DirectoryEntry> LoadEntitlements(
        const DirectoryEntry& account) override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<DirectoryEntry>,
        VirtualAdministrationClient, "load_entitlements", LoadEntitlements,
        account);
    }

    void StoreEntitlements(const DirectoryEntry& account,
        const std::vector<DirectoryEntry>& entitlements) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualAdministrationClient,
        "store_entitlements", StoreEntitlements, account, entitlements);
    }

    const Publisher<RiskParameters>& GetRiskParametersPublisher(
        const DirectoryEntry& account) override {
      PYBIND11_OVERLOAD_PURE_NAME(const Publisher<RiskParameters>&,
        VirtualAdministrationClient, "get_risk_parameters_publisher",
        GetRiskParametersPublisher, account);
    }

    void StoreRiskParameters(const DirectoryEntry& account,
        const RiskParameters& riskParameters) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualAdministrationClient,
        "store_risk_parameters", StoreRiskParameters, account, riskParameters);
    }

    const Publisher<RiskState>& GetRiskStatePublisher(
        const DirectoryEntry& account) override {
      PYBIND11_OVERLOAD_PURE_NAME(const Publisher<RiskState>&,
        VirtualAdministrationClient, "get_risk_state_publisher",
        GetRiskStatePublisher, account);
    }

    void StoreRiskState(const DirectoryEntry& account,
        const RiskState& riskState) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualAdministrationClient,
        "store_risk_state", StoreRiskState, account, riskState);
    }

    AccountModificationRequest LoadAccountModificationRequest(
        AccountModificationRequest::Id id) override {
      PYBIND11_OVERLOAD_PURE_NAME(AccountModificationRequest,
        VirtualAdministrationClient, "load_account_modification_request",
        LoadAccountModificationRequest, id);
    }

    std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(const DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<AccountModificationRequest::Id>,
        VirtualAdministrationClient, "load_account_modification_request_ids",
        LoadAccountModificationRequestIds, account, startId, maxCount);
    }

    std::vector<AccountModificationRequest::Id>
        LoadManagedAccountModificationRequestIds(const DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<AccountModificationRequest::Id>,
        VirtualAdministrationClient,
        "load_managed_account_modification_request_ids",
        LoadManagedAccountModificationRequestIds, account, startId, maxCount);
    }

    EntitlementModification LoadEntitlementModification(
        AccountModificationRequest::Id id) override {
      PYBIND11_OVERLOAD_PURE_NAME(EntitlementModification,
        VirtualAdministrationClient, "load_entitlement_modification",
        LoadEntitlementModification, id);
    }

    AccountModificationRequest SubmitAccountModificationRequest(
        const DirectoryEntry& account,
        const EntitlementModification& modification,
        const AdministrationService::Message& comment) override {
      PYBIND11_OVERLOAD_PURE_NAME(AccountModificationRequest,
        VirtualAdministrationClient, "submit_account_modification_request",
        SubmitAccountModificationRequest, account, modification, comment);
    }

    RiskModification LoadRiskModification(
        AccountModificationRequest::Id id) override {
      PYBIND11_OVERLOAD_PURE_NAME(RiskModification, VirtualAdministrationClient,
        "load_risk_modification", LoadRiskModification, id);
    }

    AccountModificationRequest SubmitAccountModificationRequest(
        const DirectoryEntry& account, const RiskModification& modification,
        const AdministrationService::Message& comment) override {
      PYBIND11_OVERLOAD_PURE_NAME(AccountModificationRequest,
        VirtualAdministrationClient, "submit_account_modification_request",
        SubmitAccountModificationRequest, account, modification, comment);
    }

    AccountModificationRequest::Update LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id) override {
      PYBIND11_OVERLOAD_PURE_NAME(AccountModificationRequest::Update,
        VirtualAdministrationClient, "load_account_modification_request_status",
        LoadAccountModificationRequestStatus, id);
    }

    AccountModificationRequest::Update ApproveAccountModificationRequest(
        AccountModificationRequest::Id id,
        const AdministrationService::Message& comment) override {
      PYBIND11_OVERLOAD_PURE_NAME(AccountModificationRequest::Update,
        VirtualAdministrationClient, "approve_account_modification_request",
        ApproveAccountModificationRequest, id, comment);
    }

    AccountModificationRequest::Update RejectAccountModificationRequest(
        AccountModificationRequest::Id id,
        const AdministrationService::Message& comment) override {
      PYBIND11_OVERLOAD_PURE_NAME(AccountModificationRequest::Update,
        VirtualAdministrationClient, "reject_account_modification_request",
        RejectAccountModificationRequest, id, comment);
    }

    AdministrationService::Message LoadMessage(
        AdministrationService::Message::Id id) override {
      PYBIND11_OVERLOAD_PURE_NAME(AdministrationService::Message,
        VirtualAdministrationClient, "load_message", LoadMessage, id);
    }

    std::vector<AdministrationService::Message::Id> LoadMessageIds(
        AccountModificationRequest::Id id) override {
      PYBIND11_OVERLOAD_PURE_NAME(
        std::vector<AdministrationService::Message::Id>,
        VirtualAdministrationClient, "load_message_ids", LoadMessageIds, id);
    }

    AdministrationService::Message SendAccountModificationRequestMessage(
        AccountModificationRequest::Id id,
        const AdministrationService::Message& message) override {
      PYBIND11_OVERLOAD_PURE_NAME(AdministrationService::Message,
        VirtualAdministrationClient,
        "send_account_modification_request_message",
        SendAccountModificationRequestMessage, id, message);
    }

    void Open() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualAdministrationClient, "open",
        Open);
    }

    void Close() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualAdministrationClient, "close",
        Close);
    }
  };
}

void Nexus::Python::ExportAccountIdentity(pybind11::module& module) {
  class_<AccountIdentity>(module, "AccountIdentity")
    .def(init())
    .def(init<const AccountIdentity&>())
    .def_readwrite("registration_time", &AccountIdentity::m_registrationTime)
    .def_readwrite("last_login_time", &AccountIdentity::m_lastLoginTime)
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

void Nexus::Python::ExportAccountModificationRequest(pybind11::module& module) {
  auto outer = class_<AccountModificationRequest>(module,
    "AccountModificationRequest")
    .def(init<AccountModificationRequest::Id, AccountModificationRequest::Type,
      DirectoryEntry, DirectoryEntry, ptime>())
    .def(init<const AccountModificationRequest&>())
    .def_property_readonly("id", &AccountModificationRequest::GetId)
    .def_property_readonly("type", &AccountModificationRequest::GetType)
    .def_property_readonly("account", &AccountModificationRequest::GetAccount)
    .def_property_readonly("submission_account",
      &AccountModificationRequest::GetSubmissionAccount)
    .def_property_readonly("timestamp",
      &AccountModificationRequest::GetTimestamp);
  enum_<AccountModificationRequest::Type>(outer, "Type")
    .value("ENTITLEMENTS", AccountModificationRequest::Type::ENTITLEMENTS);
  enum_<AccountModificationRequest::Status>(outer, "Status")
    .value("NONE", AccountModificationRequest::Status::NONE)
    .value("PENDING", AccountModificationRequest::Status::PENDING)
    .value("REVIEWED", AccountModificationRequest::Status::REVIEWED)
    .value("SCHEDULED", AccountModificationRequest::Status::SCHEDULED)
    .value("GRANTED", AccountModificationRequest::Status::GRANTED)
    .value("REJECTED", AccountModificationRequest::Status::REJECTED);
  class_<AccountModificationRequest::Update>(outer, "Update")
    .def(init())
    .def(init<AccountModificationRequest::Status, DirectoryEntry, int, ptime>())
    .def_readwrite("status", &AccountModificationRequest::Update::m_status)
    .def_readwrite("account", &AccountModificationRequest::Update::m_account)
    .def_readwrite("sequence_number",
      &AccountModificationRequest::Update::m_sequenceNumber)
    .def_readwrite("timestamp",
      &AccountModificationRequest::Update::m_timestamp);
}

void Nexus::Python::ExportAdministrationClient(pybind11::module& module) {
  class_<VirtualAdministrationClient, TrampoilineAdministrationClient>(module,
      "AdministrationClient")
    .def("load_accounts_by_roles",
      &VirtualAdministrationClient::LoadAccountsByRoles)
    .def("load_administrators_root_entry",
      &VirtualAdministrationClient::LoadAdministratorsRootEntry)
    .def("load_services_root_entry",
      &VirtualAdministrationClient::LoadServicesRootEntry)
    .def("load_trading_groups_root_entry",
      &VirtualAdministrationClient::LoadTradingGroupsRootEntry)
    .def("check_administrator",
      &VirtualAdministrationClient::CheckAdministrator)
    .def("load_account_roles",
      static_cast<AccountRoles (VirtualAdministrationClient::*)(
      const DirectoryEntry&)>(&VirtualAdministrationClient::LoadAccountRoles))
    .def("load_account_roles",
      static_cast<AccountRoles (VirtualAdministrationClient::*)(
      const DirectoryEntry&, const DirectoryEntry&)>(
      &VirtualAdministrationClient::LoadAccountRoles))
    .def("load_trading_group_entry",
      &VirtualAdministrationClient::LoadTradingGroupEntry)
    .def("load_identity", &VirtualAdministrationClient::LoadIdentity)
    .def("store_identity", &VirtualAdministrationClient::StoreIdentity)
    .def("load_trading_group", &VirtualAdministrationClient::LoadTradingGroup)
    .def("load_managed_trading_groups",
      &VirtualAdministrationClient::LoadManagedTradingGroups)
    .def("load_administrators",
      &VirtualAdministrationClient::LoadAdministrators)
    .def("load_services", &VirtualAdministrationClient::LoadServices)
    .def("load_entitlements",
      static_cast<EntitlementDatabase (VirtualAdministrationClient::*)()>(
      &VirtualAdministrationClient::LoadEntitlements))
    .def("load_entitlements",
      static_cast<std::vector<DirectoryEntry> (VirtualAdministrationClient::*)(
      const DirectoryEntry&)>(&VirtualAdministrationClient::LoadEntitlements))
    .def("store_entitlements", &VirtualAdministrationClient::StoreEntitlements)
    .def("get_risk_parameters_publisher",
      &VirtualAdministrationClient::GetRiskParametersPublisher,
      return_value_policy::reference_internal)
    .def("store_risk_parameters",
      &VirtualAdministrationClient::StoreRiskParameters)
    .def("get_risk_state_publisher",
      &VirtualAdministrationClient::GetRiskStatePublisher,
      return_value_policy::reference_internal)
    .def("store_risk_state", &VirtualAdministrationClient::StoreRiskState)
    .def("load_account_modification_request",
      &VirtualAdministrationClient::LoadAccountModificationRequest)
    .def("load_account_modification_request_ids",
      &VirtualAdministrationClient::LoadAccountModificationRequestIds)
    .def("load_managed_account_modification_request_ids",
      &VirtualAdministrationClient::LoadManagedAccountModificationRequestIds)
    .def("load_entitlement_modification",
      &VirtualAdministrationClient::LoadEntitlementModification)
    .def("submit_account_modification_request", static_cast<
      AccountModificationRequest (VirtualAdministrationClient::*)(
      const DirectoryEntry&, const EntitlementModification&,
      const AdministrationService::Message&)>(
      &VirtualAdministrationClient::SubmitAccountModificationRequest))
    .def("load_risk_modification",
      &VirtualAdministrationClient::LoadRiskModification)
    .def("submit_account_modification_request", static_cast<
      AccountModificationRequest (VirtualAdministrationClient::*)(
      const DirectoryEntry&, const RiskModification&,
      const AdministrationService::Message&)>(
      &VirtualAdministrationClient::SubmitAccountModificationRequest))
    .def("load_account_modification_request_status",
      &VirtualAdministrationClient::LoadAccountModificationRequestStatus)
    .def("approve_account_modification_request",
      &VirtualAdministrationClient::ApproveAccountModificationRequest)
    .def("reject_account_modification_request",
      &VirtualAdministrationClient::RejectAccountModificationRequest)
    .def("load_message", &VirtualAdministrationClient::LoadMessage)
    .def("load_message_ids", &VirtualAdministrationClient::LoadMessageIds)
    .def("send_account_modification_request_message",
      &VirtualAdministrationClient::SendAccountModificationRequestMessage)
    .def("open", &VirtualAdministrationClient::Open)
    .def("close", &VirtualAdministrationClient::Close);
}

void Nexus::Python::ExportAdministrationService(pybind11::module& module) {
  auto submodule = module.def_submodule("administration_service");
  ExportAccountIdentity(submodule);
  ExportAccountModificationRequest(submodule);
  ExportAdministrationClient(submodule);
  ExportApplicationAdministrationClient(submodule);
  ExportEntitlementModification(submodule);
  ExportMessage(submodule);
  ExportTradingGroup(submodule);
  ExportQueueSuite<RiskState>(submodule, "RiskState");
  ExportQueueSuite<RiskParameters>(submodule, "RiskParameters");
  auto test_module = submodule.def_submodule("tests");
  ExportAdministrationServiceTestEnvironment(test_module);
}

void Nexus::Python::ExportAdministrationServiceTestEnvironment(
    pybind11::module& module) {
  class_<AdministrationServiceTestEnvironment>(module,
      "AdministrationServiceTestEnvironment")
    .def(init(
      [] (std::shared_ptr<VirtualServiceLocatorClient> serviceLocatorClient) {
        return std::make_unique<AdministrationServiceTestEnvironment>(
          std::move(serviceLocatorClient));
      }))
    .def("open", &AdministrationServiceTestEnvironment::Open,
      call_guard<GilRelease>())
    .def("close", &AdministrationServiceTestEnvironment::Close,
      call_guard<GilRelease>())
    .def("make_administrator",
      &AdministrationServiceTestEnvironment::MakeAdministrator,
      call_guard<GilRelease>())
    .def("build_client",
      [] (AdministrationServiceTestEnvironment& self,
          VirtualServiceLocatorClient& serviceLocatorClient) {
        return MakeToPythonAdministrationClient(self.BuildClient(
          Ref(serviceLocatorClient)));
      });
}

void Nexus::Python::ExportApplicationAdministrationClient(
    pybind11::module& module) {
  using SessionBuilder = AuthenticatedServiceProtocolClientBuilder<
    VirtualServiceLocatorClient, MessageProtocol<
    std::unique_ptr<TcpSocketChannel>, BinarySender<SharedBuffer>, NullEncoder>,
    LiveTimer>;
  using Client = AdministrationClient<SessionBuilder>;
  class_<ToPythonAdministrationClient<Client>, VirtualAdministrationClient>(
      module, "ApplicationAdministrationClient")
    .def(init(
      [] (VirtualServiceLocatorClient& serviceLocatorClient) {
        auto addresses = LocateServiceAddresses(serviceLocatorClient,
          AdministrationService::SERVICE_NAME);
        auto delay = false;
        auto sessionBuilder = SessionBuilder(Ref(serviceLocatorClient),
          [=] () mutable {
            if(delay) {
              auto delayTimer = LiveTimer(seconds(3),
                Ref(*GetTimerThreadPool()));
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
          sessionBuilder));
      }));
}

void Nexus::Python::ExportEntitlementModification(pybind11::module& module) {
  class_<EntitlementModification>(module, "EntitlementModification")
    .def(init())
    .def(init<std::vector<DirectoryEntry>>())
    .def(init<const EntitlementModification&>())
    .def_property_readonly("entitlements",
      &EntitlementModification::GetEntitlements);
}

void Nexus::Python::ExportMessage(pybind11::module& module) {
  auto outer = class_<AdministrationService::Message>(module, "Message")
    .def(init())
    .def(init<AdministrationService::Message::Id, DirectoryEntry, ptime,
      std::vector<AdministrationService::Message::Body>>())
    .def(init<const AdministrationService::Message&>())
    .def_property_readonly("id", &AdministrationService::Message::GetId)
    .def_property_readonly("account",
      &AdministrationService::Message::GetAccount)
    .def_property_readonly("timestamp",
      &AdministrationService::Message::GetTimestamp)
    .def_property_readonly("body", &AdministrationService::Message::GetBody)
    .def_property_readonly("bodies",
      &AdministrationService::Message::GetBodies);
  class_<AdministrationService::Message::Body>(outer, "Body")
    .def(init())
    .def(init<std::string, std::string>())
    .def(init<const AdministrationService::Message::Body&>())
    .def_property_readonly_static("EMPTY",
      &AdministrationService::Message::Body::EMPTY)
    .def_static("make_plain_text",
      &AdministrationService::Message::Body::MakePlainText)
    .def_readwrite("content_type",
      &AdministrationService::Message::Body::m_contentType)
    .def_readwrite("message",
      &AdministrationService::Message::Body::m_message)
    .def(self == self)
    .def(self != self);
}

void Nexus::Python::ExportTradingGroup(pybind11::module& module) {
  class_<TradingGroup>(module, "TradingGroup")
    .def(init())
    .def(init<const DirectoryEntry&, const DirectoryEntry&,
      const std::vector<DirectoryEntry>&, const DirectoryEntry&,
      const std::vector<DirectoryEntry>&>())
    .def(init<const TradingGroup&>())
    .def_property_readonly("entry", &TradingGroup::GetEntry)
    .def_property_readonly("managers", &TradingGroup::GetManagers)
    .def_property_readonly("traders", &TradingGroup::GetTraders);
}
