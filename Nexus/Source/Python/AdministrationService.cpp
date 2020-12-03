#include "Nexus/Python/AdministrationService.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Python/ToPythonAdministrationClient.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::Python;
using namespace Nexus::RiskService;
using namespace pybind11;

namespace {
  auto administrationClientBox =
    std::unique_ptr<class_<AdministrationClientBox>>();
}

class_<AdministrationClientBox>&
    Nexus::Python::GetExportedAdministrationClientBox() {
  return *administrationClientBox;
}

void Nexus::Python::ExportAccountIdentity(module& module) {
  class_<AccountIdentity>(module, "AccountIdentity").
    def(init()).
    def(init<const AccountIdentity&>()).
    def_readwrite("registration_time", &AccountIdentity::m_registrationTime).
    def_readwrite("last_login_time", &AccountIdentity::m_lastLoginTime).
    def_readwrite("first_name", &AccountIdentity::m_firstName).
    def_readwrite("last_name", &AccountIdentity::m_lastName).
    def_readwrite("email_address", &AccountIdentity::m_emailAddress).
    def_readwrite("address_line_one", &AccountIdentity::m_addressLineOne).
    def_readwrite("address_line_two", &AccountIdentity::m_addressLineTwo).
    def_readwrite("address_line_three", &AccountIdentity::m_addressLineThree).
    def_readwrite("city", &AccountIdentity::m_city).
    def_readwrite("province", &AccountIdentity::m_province).
    def_readwrite("country", &AccountIdentity::m_country).
    def_readwrite("photo_id", &AccountIdentity::m_photoId).
    def_readwrite("user_notes", &AccountIdentity::m_userNotes);
}

void Nexus::Python::ExportAccountModificationRequest(module& module) {
  auto outer = class_<AccountModificationRequest>(module,
    "AccountModificationRequest").
    def(init<AccountModificationRequest::Id, AccountModificationRequest::Type,
      DirectoryEntry, DirectoryEntry, ptime>()).
    def(init<const AccountModificationRequest&>()).
    def_property_readonly("id", &AccountModificationRequest::GetId).
    def_property_readonly("type", &AccountModificationRequest::GetType).
    def_property_readonly("account", &AccountModificationRequest::GetAccount).
    def_property_readonly("submission_account",
      &AccountModificationRequest::GetSubmissionAccount).
    def_property_readonly("timestamp",
      &AccountModificationRequest::GetTimestamp);
  enum_<AccountModificationRequest::Type>(outer, "Type").
    value("ENTITLEMENTS", AccountModificationRequest::Type::ENTITLEMENTS).
    value("RISK", AccountModificationRequest::Type::RISK);
  enum_<AccountModificationRequest::Status>(outer, "Status").
    value("NONE", AccountModificationRequest::Status::NONE).
    value("PENDING", AccountModificationRequest::Status::PENDING).
    value("REVIEWED", AccountModificationRequest::Status::REVIEWED).
    value("SCHEDULED", AccountModificationRequest::Status::SCHEDULED).
    value("GRANTED", AccountModificationRequest::Status::GRANTED).
    value("REJECTED", AccountModificationRequest::Status::REJECTED);
  class_<AccountModificationRequest::Update>(outer, "Update").
    def(init()).
    def(init<AccountModificationRequest::Status, DirectoryEntry, int, ptime>()).
    def_readwrite("status", &AccountModificationRequest::Update::m_status).
    def_readwrite("account", &AccountModificationRequest::Update::m_account).
    def_readwrite("sequence_number",
      &AccountModificationRequest::Update::m_sequenceNumber).
    def_readwrite("timestamp",
      &AccountModificationRequest::Update::m_timestamp);
}

void Nexus::Python::ExportAdministrationService(module& module) {
  auto submodule = module.def_submodule("administration_service");
  ExportAccountIdentity(submodule);
  ExportAccountModificationRequest(submodule);
  administrationClientBox = std::make_unique<class_<AdministrationClientBox>>(
    ExportAdministrationClient<AdministrationClientBox>(submodule,
      "AdministrationClient"));
  ExportAdministrationClient<
    ToPythonAdministrationClient<AdministrationClientBox>>(submodule,
      "AdministrationClientBox");
  ExportApplicationAdministrationClient(submodule);
  ExportEntitlementModification(submodule);
  ExportMessage(submodule);
  ExportTradingGroup(submodule);
  ExportQueueSuite<RiskState>(submodule, "RiskState");
  ExportQueueSuite<RiskParameters>(submodule, "RiskParameters");
  auto test_module = submodule.def_submodule("tests");
  ExportAdministrationServiceTestEnvironment(test_module);
}

void Nexus::Python::ExportAdministrationServiceTestEnvironment(module& module) {
  class_<AdministrationServiceTestEnvironment>(module,
      "AdministrationServiceTestEnvironment").
    def(init<ServiceLocatorClientBox>(), call_guard<GilRelease>()).
    def("__del__",
      [] (AdministrationServiceTestEnvironment& self) {
        self.Close();
      }, call_guard<GilRelease>()).
    def("close", &AdministrationServiceTestEnvironment::Close,
      call_guard<GilRelease>()).
    def("make_administrator",
      &AdministrationServiceTestEnvironment::MakeAdministrator,
      call_guard<GilRelease>()).
    def("make_client",
      [] (AdministrationServiceTestEnvironment& self,
          ServiceLocatorClientBox serviceLocatorClient) {
        return ToPythonAdministrationClient(self.MakeClient(
          std::move(serviceLocatorClient)));
      }, call_guard<GilRelease>());
}

void Nexus::Python::ExportApplicationAdministrationClient(module& module) {
  using PythonApplicationAdministrationClient = ToPythonAdministrationClient<
    AdministrationClient<DefaultSessionBuilder<ServiceLocatorClientBox>>>;
  ExportAdministrationClient<PythonApplicationAdministrationClient>(module,
    "ApplicationAdministrationClient").
    def(init([] (ServiceLocatorClientBox serviceLocatorClient) {
      return std::make_shared<PythonApplicationAdministrationClient>(
        MakeDefaultSessionBuilder(std::move(serviceLocatorClient),
          AdministrationService::SERVICE_NAME));
    }));
}

void Nexus::Python::ExportEntitlementModification(module& module) {
  class_<EntitlementModification>(module, "EntitlementModification").
    def(init()).
    def(init<std::vector<DirectoryEntry>>()).
    def(init<const EntitlementModification&>()).
    def_property_readonly("entitlements",
      &EntitlementModification::GetEntitlements);
}

void Nexus::Python::ExportMessage(module& module) {
  auto outer = class_<AdministrationService::Message>(module, "Message").
    def(init()).
    def(init<AdministrationService::Message::Id, DirectoryEntry, ptime,
      std::vector<AdministrationService::Message::Body>>()).
    def(init<const AdministrationService::Message&>()).
    def_property_readonly("id", &AdministrationService::Message::GetId).
    def_property_readonly("account",
      &AdministrationService::Message::GetAccount).
    def_property_readonly("timestamp",
      &AdministrationService::Message::GetTimestamp).
    def_property_readonly("body", &AdministrationService::Message::GetBody).
    def_property_readonly("bodies", &AdministrationService::Message::GetBodies);
  class_<AdministrationService::Message::Body>(outer, "Body").
    def(init()).
    def(init<std::string, std::string>()).
    def(init<const AdministrationService::Message::Body&>()).
    def_property_readonly_static("EMPTY",
      &AdministrationService::Message::Body::EMPTY).
    def_static("make_plain_text",
      &AdministrationService::Message::Body::MakePlainText).
    def_readwrite("content_type",
      &AdministrationService::Message::Body::m_contentType).
    def_readwrite("message",
      &AdministrationService::Message::Body::m_message).
    def(self == self).
    def(self != self);
}

void Nexus::Python::ExportTradingGroup(module& module) {
  class_<TradingGroup>(module, "TradingGroup").
    def(init()).
    def(init<const DirectoryEntry&, const DirectoryEntry&,
      const std::vector<DirectoryEntry>&, const DirectoryEntry&,
      const std::vector<DirectoryEntry>&>()).
    def(init<const TradingGroup&>()).
    def_property_readonly("entry", &TradingGroup::GetEntry).
    def_property_readonly("managers", &TradingGroup::GetManagers).
    def_property_readonly("traders", &TradingGroup::GetTraders);
}
