#include "Nexus/Python/AdministrationService.hpp"
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/PythonBindings.hpp>
#include <Beam/Python/Queues.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClientBuilder.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/AdministrationService/TradingGroup.hpp"
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestInstance.hpp"

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

  VirtualAdministrationClient* BuildClient(
      VirtualServiceLocatorClient& serviceLocatorClient) {
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
    auto baseClient = std::make_unique<Client>(sessionBuilder);
    return new WrapperAdministrationClient<std::unique_ptr<Client>>{
      std::move(baseClient)};
  }
}

void Nexus::Python::ExportAccountIdentity() {
  class_<AccountIdentity>("AccountIdentity", init<>())
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

void Nexus::Python::ExportAdministrationClient() {
  class_<VirtualAdministrationClient, boost::noncopyable>(
    "AdministrationClient", no_init)
    .def("__init__", make_constructor(&BuildClient))
    .def("check_administrator",
      BlockingFunction(&VirtualAdministrationClient::CheckAdministrator))
    .def("load_trading_group_entry",
      BlockingFunction(&VirtualAdministrationClient::LoadTradingGroupEntry))
    .def("load_identity", BlockingFunction(
      &VirtualAdministrationClient::LoadIdentity))
    .def("store_identity", BlockingFunction(
      &VirtualAdministrationClient::StoreIdentity))
    .def("load_trading_group", BlockingFunction(
      &VirtualAdministrationClient::LoadTradingGroup))
    .def("load_managed_trading_groups",
      BlockingFunction(&VirtualAdministrationClient::LoadManagedTradingGroups))
    .def("load_administrators",
      BlockingFunction(&VirtualAdministrationClient::LoadAdministrators))
    .def("load_services",
      BlockingFunction(&VirtualAdministrationClient::LoadServices))
    .def("load_entitlements", BlockingFunction(
        static_cast<EntitlementDatabase (VirtualAdministrationClient::*)()>(
        &VirtualAdministrationClient::LoadEntitlements)))
    .def("load_entitlements", BlockingFunction(
      static_cast<vector<DirectoryEntry> (VirtualAdministrationClient::*)(
      const DirectoryEntry&)>(&VirtualAdministrationClient::LoadEntitlements)))
    .def("store_entitlements",
      BlockingFunction(&VirtualAdministrationClient::StoreEntitlements))
    .def("get_risk_parameters_publisher", BlockingFunction(
      &VirtualAdministrationClient::GetRiskParametersPublisher,
      return_value_policy<reference_existing_object>()))
    .def("store_risk_parameters", BlockingFunction(
      &VirtualAdministrationClient::StoreRiskParameters))
    .def("get_risk_state_publisher", BlockingFunction(
      &VirtualAdministrationClient::GetRiskStatePublisher,
      return_value_policy<reference_existing_object>()))
    .def("store_risk_state", BlockingFunction(
      &VirtualAdministrationClient::StoreRiskState))
    .def("open", BlockingFunction(&VirtualAdministrationClient::Open))
    .def("close", BlockingFunction(&VirtualAdministrationClient::Close));
  ExportPublisher<RiskState>("RiskStatePublisher");
  ExportPublisher<RiskParameters>("RiskParametersPublisher");
}

void Nexus::Python::ExportAdministrationService() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".administration_service");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("administration_service") = nestedModule;
  scope parent = nestedModule;
  ExportAccountIdentity();
  ExportAdministrationClient();
  ExportTradingGroup();
  {
    string nestedName = extract<string>(parent.attr("__name__") + ".tests");
    object nestedModule{handle<>(
      borrowed(PyImport_AddModule(nestedName.c_str())))};
    parent.attr("tests") = nestedModule;
    scope child = nestedModule;
    ExportAdministrationServiceTestInstance();
  }
}

void Nexus::Python::ExportAdministrationServiceTestInstance() {
  class_<AdministrationServiceTestInstance, boost::noncopyable>(
      "AdministrationServiceTestInstance",
      init<const std::shared_ptr<VirtualServiceLocatorClient>&>())
    .def("open", BlockingFunction(&AdministrationServiceTestInstance::Open))
    .def("close", BlockingFunction(&AdministrationServiceTestInstance::Close))
    .def("build_client",
      ReleaseUniquePtr(&AdministrationServiceTestInstance::BuildClient));
}

void Nexus::Python::ExportTradingGroup() {
  class_<TradingGroup>("TradingGroup", init<>())
    .def(init<const DirectoryEntry&, const DirectoryEntry&,
      const vector<DirectoryEntry>&, const DirectoryEntry&,
      const vector<DirectoryEntry>&>())
    .add_property("entry", make_function(&TradingGroup::GetEntry,
      return_value_policy<return_by_value>()))
    .add_property("managers", make_function(&TradingGroup::GetManagers,
      return_value_policy<return_by_value>()))
    .add_property("traders", make_function(&TradingGroup::GetTraders,
      return_value_policy<return_by_value>()));
}
