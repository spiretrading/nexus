#include "Nexus/Python/ServiceClients.hpp"
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/PythonBindings.hpp>
#include <Beam/Threading/VirtualTimer.hpp>
#include <Beam/TimeService/VirtualTimeClient.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/ServiceClients/ApplicationServiceClients.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace std;

namespace {
  struct VirtualServiceClientsWrapper :
      VirtualServiceClients, wrapper<VirtualServiceClients> {
    virtual ServiceLocatorClient& GetServiceLocatorClient() override {
      return *static_cast<ServiceLocatorClient*>(
        this->get_override("get_service_locator_client")());
    }

    virtual RegistryClient& GetRegistryClient() override {
      return *static_cast<RegistryClient*>(
        this->get_override("get_registry_client")());
    }

    virtual AdministrationClient& GetAdministrationClient() override {
      return *static_cast<AdministrationClient*>(
        this->get_override("get_administration_client")());
    }

    virtual DefinitionsClient& GetDefinitionsClient() override {
      return *static_cast<DefinitionsClient*>(
        this->get_override("get_definitions_client")());
    }

    virtual MarketDataClient& GetMarketDataClient() override {
      return *static_cast<MarketDataClient*>(
        this->get_override("get_market_data_client")());
    }

    virtual ChartingClient& GetChartingClient() override {
      return *static_cast<ChartingClient*>(
        this->get_override("get_charting_client")());
    }

    virtual ComplianceClient& GetComplianceClient() override {
      return *static_cast<ComplianceClient*>(
        this->get_override("get_compliance_client")());
    }

    virtual OrderExecutionClient& GetOrderExecutionClient() override {
      return *static_cast<OrderExecutionClient*>(
        this->get_override("get_order_execution_client")());
    }

    virtual RiskClient& GetRiskClient() override {
      return *static_cast<RiskClient*>(this->get_override("get_risk_client")());
    }

    virtual TimeClient& GetTimeClient() override {
      return *static_cast<TimeClient*>(this->get_override("get_time_client")());
    }

    virtual std::unique_ptr<Timer> BuildTimer(
        boost::posix_time::time_duration expiry) override {
      return std::unique_ptr<Timer>{
        static_cast<Timer*>(this->get_override("build_timer")())};
    }

    virtual void Open() override {
      this->get_override("open")();
    }

    virtual void Close() override {
      this->get_override("close")();
    }
  };
}

#ifdef _MSC_VER
namespace boost {
  template<> inline const volatile
      ChartingService::VirtualChartingClient* get_pointer(
      const volatile ChartingService::VirtualChartingClient* p) {
    return p;
  }

  template<> inline const volatile
      Compliance::VirtualComplianceClient* get_pointer(
      const volatile Compliance::VirtualComplianceClient* p) {
    return p;
  }

  template<> inline const volatile
      DefinitionsService::VirtualDefinitionsClient* get_pointer(
      const volatile DefinitionsService::VirtualDefinitionsClient* p) {
    return p;
  }

  template<> inline const volatile VirtualMarketDataClient* get_pointer(
      const volatile VirtualMarketDataClient* p) {
    return p;
  }

  template<> inline const volatile
      OrderExecutionService::VirtualOrderExecutionClient* get_pointer(
      const volatile OrderExecutionService::VirtualOrderExecutionClient* p) {
    return p;
  }

  template<> inline const volatile
      RegistryService::VirtualRegistryClient* get_pointer(
      const volatile RegistryService::VirtualRegistryClient* p) {
    return p;
  }

  template<> inline const volatile RiskService::VirtualRiskClient* get_pointer(
      const volatile RiskService::VirtualRiskClient* p) {
    return p;
  }

  template<> inline const volatile
      ServiceLocator::VirtualServiceLocatorClient* get_pointer(
      const volatile ServiceLocator::VirtualServiceLocatorClient* p) {
    return p;
  }

  template<> inline const volatile VirtualTimeClient* get_pointer(
      const volatile VirtualTimeClient* p) {
    return p;
  }

  template<> inline const volatile VirtualTimer* get_pointer(
      const volatile VirtualTimer* p) {
    return p;
  }
}
#endif

void Nexus::Python::ExportApplicationServiceClients() {
  class_<ApplicationServiceClients, boost::noncopyable,
    bases<VirtualServiceClients>>("ApplicationServiceClients", no_init)
    .def("get_service_locator_client", BlockingFunction(
      &ApplicationServiceClients::GetServiceLocatorClient,
      return_internal_reference<>()))
    .def("get_registry_client", BlockingFunction(
      &ApplicationServiceClients::GetRegistryClient,
      return_internal_reference<>()))
    .def("get_administration_client", BlockingFunction(
      &ApplicationServiceClients::GetAdministrationClient,
      return_internal_reference<>()))
    .def("get_definitions_client", BlockingFunction(
      &ApplicationServiceClients::GetDefinitionsClient,
      return_internal_reference<>()))
    .def("get_market_data_client", BlockingFunction(
      &ApplicationServiceClients::GetMarketDataClient,
      return_internal_reference<>()))
    .def("get_charting_client", BlockingFunction(
      &ApplicationServiceClients::GetChartingClient,
      return_internal_reference<>()))
    .def("get_compliance_client", BlockingFunction(
      &ApplicationServiceClients::GetComplianceClient,
      return_internal_reference<>()))
    .def("get_order_execution_client", BlockingFunction(
      &ApplicationServiceClients::GetOrderExecutionClient,
      return_internal_reference<>()))
    .def("get_risk_client", BlockingFunction(
      &ApplicationServiceClients::GetRiskClient, return_internal_reference<>()))
    .def("get_time_client", BlockingFunction(
      &ApplicationServiceClients::GetTimeClient, return_internal_reference<>()))
    .def("build_timer", ReleaseUniquePtr<ApplicationServiceClients>(
      &ApplicationServiceClients::BuildTimer))
    .def("open", BlockingFunction(&ApplicationServiceClients::Open))
    .def("close", BlockingFunction(&ApplicationServiceClients::Close));
}

void Nexus::Python::ExportServiceClients() {
  ExportVirtualServiceClients();
  ExportApplicationServiceClients();
  ExportTestEnvironment();
  ExportTestServiceClients();
}

void Nexus::Python::ExportTestEnvironment() {
  class_<TestEnvironment, std::shared_ptr<TestEnvironment>, boost::noncopyable>(
      "TestEnvironment", init<>())
    .def("set_time", BlockingFunction(&TestEnvironment::SetTime))
    .def("advance_time", BlockingFunction(&TestEnvironment::AdvanceTime))
    .def("publish", BlockingFunction(static_cast<void (TestEnvironment::*)(
      MarketCode, const OrderImbalance&)>(&TestEnvironment::Publish)))
    .def("publish", BlockingFunction(static_cast<void (TestEnvironment::*)(
      const Security&, const BboQuote&)>(&TestEnvironment::Publish)))
    .def("publish", BlockingFunction(
      static_cast<void (TestEnvironment::*)(const Security&, const BookQuote&)>(
      &TestEnvironment::Publish)))
    .def("publish", BlockingFunction(
      static_cast<void (TestEnvironment::*)(
      const Security&, const MarketQuote&)>(&TestEnvironment::Publish)))
    .def("publish", BlockingFunction(
      static_cast<void (TestEnvironment::*)(
      const Security&, const TimeAndSale&)>(&TestEnvironment::Publish)))
    .def("update_bbo_price", BlockingFunction(
      static_cast<void (TestEnvironment::*)(const Security&, Money, Money)>(
      &TestEnvironment::UpdateBboPrice)))
    .def("update_bbo_price", BlockingFunction(
      static_cast<void (TestEnvironment::*)(const Security&, Money, Money,
      const ptime&)>(&TestEnvironment::UpdateBboPrice)))
    .def("monitor_order_submissions",
      BlockingFunction(&TestEnvironment::MonitorOrderSubmissions))
    .def("accept_order", BlockingFunction(&TestEnvironment::AcceptOrder))
    .def("reject_order", BlockingFunction(&TestEnvironment::RejectOrder))
    .def("cancel_order", BlockingFunction(&TestEnvironment::CancelOrder))
    .def("fill_order", BlockingFunction(
      static_cast<void (TestEnvironment::*)(const Order&, Money, Quantity)>(
      &TestEnvironment::FillOrder)))
    .def("fill_order", BlockingFunction(
      static_cast<void (TestEnvironment::*)(const Order&, Quantity)>(
      &TestEnvironment::FillOrder)))
    .def("update", BlockingFunction(
      static_cast<void (TestEnvironment::*)(const Order&,
      const ExecutionReport&)>(&TestEnvironment::Update)))
    .def("get_time_environment", &TestEnvironment::GetTimeEnvironment,
      return_internal_reference<>())
    .def("get_service_locator_environment",
      &TestEnvironment::GetServiceLocatorEnvironment,
      return_internal_reference<>())
    .def("get_uid_environment", &TestEnvironment::GetUidEnvironment,
      return_internal_reference<>())
    .def("get_administration_environment",
      &TestEnvironment::GetAdministrationEnvironment,
      return_internal_reference<>())
    .def("get_market_data_environment",
      &TestEnvironment::GetMarketDataEnvironment, return_internal_reference<>())
    .def("get_order_execution_environment",
      &TestEnvironment::GetOrderExecutionEnvironment,
      return_internal_reference<>())
    .def("open", BlockingFunction(&TestEnvironment::Open))
    .def("close", BlockingFunction(&TestEnvironment::Close));
}

void Nexus::Python::ExportTestServiceClients() {
  class_<TestServiceClients, boost::noncopyable, bases<VirtualServiceClients>>(
    "TestServiceClients", no_init)
    .def("get_service_locator_client", BlockingFunction(
      &TestServiceClients::GetServiceLocatorClient,
      return_internal_reference<>()))
    .def("get_registry_client", BlockingFunction(
      &TestServiceClients::GetRegistryClient, return_internal_reference<>()))
    .def("get_administration_client", BlockingFunction(
      &TestServiceClients::GetAdministrationClient,
      return_internal_reference<>()))
    .def("get_definitions_client", BlockingFunction(
      &TestServiceClients::GetDefinitionsClient, return_internal_reference<>()))
    .def("get_market_data_client", BlockingFunction(
      &TestServiceClients::GetMarketDataClient, return_internal_reference<>()))
    .def("get_charting_client", BlockingFunction(
      &TestServiceClients::GetChartingClient, return_internal_reference<>()))
    .def("get_compliance_client", BlockingFunction(
      &TestServiceClients::GetComplianceClient, return_internal_reference<>()))
    .def("get_order_execution_client", BlockingFunction(
      &TestServiceClients::GetOrderExecutionClient,
      return_internal_reference<>()))
    .def("get_risk_client", BlockingFunction(&TestServiceClients::GetRiskClient,
      return_internal_reference<>()))
    .def("get_time_client", BlockingFunction(&TestServiceClients::GetTimeClient,
      return_internal_reference<>()))
    .def("build_timer", ReleaseUniquePtr<TestServiceClients>(
      &TestServiceClients::BuildTimer))
    .def("open", BlockingFunction(&TestServiceClients::Open))
    .def("close", BlockingFunction(&TestServiceClients::Close));
}

void Nexus::Python::ExportVirtualServiceClients() {
  class_<VirtualServiceClientsWrapper, boost::noncopyable>("ServiceClients")
    .def("get_service_locator_client",
      pure_virtual(&VirtualServiceClients::GetServiceLocatorClient),
      return_internal_reference<>())
    .def("get_registry_client",
      pure_virtual(&VirtualServiceClients::GetRegistryClient),
      return_internal_reference<>())
    .def("get_administration_client",
      pure_virtual(&VirtualServiceClients::GetAdministrationClient),
      return_internal_reference<>())
    .def("get_definitions_client",
      pure_virtual(&VirtualServiceClients::GetDefinitionsClient),
      return_internal_reference<>())
    .def("get_market_data_client",
      pure_virtual(&VirtualServiceClients::GetMarketDataClient),
      return_internal_reference<>())
    .def("get_charting_client",
      pure_virtual(&VirtualServiceClients::GetChartingClient),
      return_internal_reference<>())
    .def("get_compliance_client",
      pure_virtual(&VirtualServiceClients::GetComplianceClient),
      return_internal_reference<>())
    .def("get_order_execution_client",
      pure_virtual(&VirtualServiceClients::GetOrderExecutionClient),
      return_internal_reference<>())
    .def("get_risk_client", pure_virtual(&VirtualServiceClients::GetRiskClient),
      return_internal_reference<>())
    .def("get_time_client", pure_virtual(&VirtualServiceClients::GetTimeClient),
      return_internal_reference<>())
    .def("build_timer", ReleaseUniquePtr(&VirtualServiceClients::BuildTimer))
    .def("open", pure_virtual(&VirtualServiceClients::Open))
    .def("close", pure_virtual(&VirtualServiceClients::Close));
}
