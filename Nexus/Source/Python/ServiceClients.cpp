#include "Nexus/Python/ServiceClients.hpp"
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/PythonBindings.hpp>
#include <Beam/Python/PythonQueueWriter.hpp>
#include <Beam/Threading/VirtualTimer.hpp>
#include <Beam/TimeService/VirtualTimeClient.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Python/PythonMarketDataClient.hpp"
#include "Nexus/Python/PythonOrderExecutionClient.hpp"
#include "Nexus/ServiceClients/ApplicationServiceClients.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"
#include "Nexus/ServiceClients/TestTimeClient.hpp"
#include "Nexus/ServiceClients/TestTimer.hpp"
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

  class PythonApplicationServiceClients :
      public WrapperServiceClients<std::unique_ptr<VirtualServiceClients>> {
    public:
      PythonApplicationServiceClients(
          std::unique_ptr<VirtualServiceClients> client)
          : WrapperServiceClients<std::unique_ptr<VirtualServiceClients>>(
              std::move(client)) {}

      virtual ~PythonApplicationServiceClients() override {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock{gil};
        Close();
      }

      virtual PythonMarketDataClient& GetMarketDataClient() override {
        if(m_marketDataClient == nullptr) {
          m_marketDataClient = std::make_unique<PythonMarketDataClient>(
            MakeVirtualMarketDataClient(
            &WrapperServiceClients<std::unique_ptr<VirtualServiceClients>>::
            GetMarketDataClient()));
        }
        return *m_marketDataClient;
      }

      virtual PythonOrderExecutionClient& GetOrderExecutionClient() override {
        if(m_orderExecutionClient == nullptr) {
          m_orderExecutionClient = std::make_unique<PythonOrderExecutionClient>(
            MakeVirtualOrderExecutionClient(
            &WrapperServiceClients<std::unique_ptr<VirtualServiceClients>>::
            GetOrderExecutionClient()));
        }
        return *m_orderExecutionClient;
      }

    private:
      std::unique_ptr<PythonMarketDataClient> m_marketDataClient;
      std::unique_ptr<PythonOrderExecutionClient> m_orderExecutionClient;
  };

  VirtualServiceClients* BuildApplicationServiceClients(
      const IpAddress& address, const string& username,
      const string& password) {
    auto baseClient = std::make_unique<ApplicationServiceClients>(
      address, username, password, Ref(*GetSocketThreadPool()),
      Ref(*GetTimerThreadPool()));
    return new PythonApplicationServiceClients{
      MakeVirtualServiceClients(std::move(baseClient))};
  }

  class PythonTestServiceClients :
      public WrapperServiceClients<std::unique_ptr<VirtualServiceClients>> {
    public:
      PythonTestServiceClients(std::unique_ptr<VirtualServiceClients> client,
          std::shared_ptr<TestEnvironment> environment)
          : WrapperServiceClients<std::unique_ptr<VirtualServiceClients>>{
              std::move(client)},
            m_environment{std::move(environment)} {}

      virtual ~PythonTestServiceClients() override {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock{gil};
        Close();
      }

      virtual PythonMarketDataClient& GetMarketDataClient() override {
        if(m_marketDataClient == nullptr) {
          m_marketDataClient = std::make_unique<PythonMarketDataClient>(
            MakeVirtualMarketDataClient(
            &WrapperServiceClients<std::unique_ptr<VirtualServiceClients>>::
            GetMarketDataClient()));
        }
        return *m_marketDataClient;
      }

      virtual PythonOrderExecutionClient& GetOrderExecutionClient() override {
        if(m_orderExecutionClient == nullptr) {
          m_orderExecutionClient = std::make_unique<PythonOrderExecutionClient>(
            MakeVirtualOrderExecutionClient(
            &WrapperServiceClients<std::unique_ptr<VirtualServiceClients>>::
            GetOrderExecutionClient()));
        }
        return *m_orderExecutionClient;
      }

    private:
      std::shared_ptr<TestEnvironment> m_environment;
      std::unique_ptr<PythonMarketDataClient> m_marketDataClient;
      std::unique_ptr<PythonOrderExecutionClient> m_orderExecutionClient;
  };

  class PythonTestTimer : public WrapperTimer<TestTimer> {
    public:
      PythonTestTimer(time_duration expiry,
          std::shared_ptr<TestEnvironment> environment)
          : WrapperTimer<TestTimer>(Initialize(expiry, Ref(*environment))),
            m_environment{std::move(environment)} {}

      virtual ~PythonTestTimer() override {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock{gil};
        Cancel();
      }

    private:
      std::shared_ptr<TestEnvironment> m_environment;
  };

  class PythonTestTimeClient : public WrapperTimeClient<TestTimeClient> {
    public:
      PythonTestTimeClient(std::shared_ptr<TestEnvironment> environment)
          : WrapperTimeClient<TestTimeClient>(Initialize(Ref(*environment))),
            m_environment{std::move(environment)} {}

      virtual ~PythonTestTimeClient() override {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock{gil};
        Close();
      }

    private:
      std::shared_ptr<TestEnvironment> m_environment;
  };

  VirtualServiceClients* BuildTestServiceClients(
      std::shared_ptr<TestEnvironment> environment) {
    auto baseClient = std::make_unique<TestServiceClients>(Ref(*environment));
    return new PythonTestServiceClients{
      MakeVirtualServiceClients(std::move(baseClient)), environment};
  }

  VirtualTimer* BuildTestTimer(time_duration expiry,
      std::shared_ptr<TestEnvironment> environment) {
    return new PythonTestTimer{expiry, environment};
  }

  VirtualTimeClient* BuildTestTimeClient(
      std::shared_ptr<TestEnvironment> environment) {
    return new PythonTestTimeClient{environment};
  }

  void TestEnvironmentMonitorOrderSubmissions(TestEnvironment& environment,
      const std::shared_ptr<PythonQueueWriter>& queue) {
    environment.MonitorOrderSubmissions(queue->GetSlot<const Order*>());
  }
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
  class_<PythonApplicationServiceClients, boost::noncopyable,
      bases<VirtualServiceClients>>("ApplicationServiceClients", no_init)
    .def("__init__", make_constructor(&BuildApplicationServiceClients))
    .def("get_service_locator_client",
      BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetServiceLocatorClient,
      return_internal_reference<>()))
    .def("get_registry_client",
      BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetRegistryClient,
      return_internal_reference<>()))
    .def("get_administration_client",
      BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetAdministrationClient,
      return_internal_reference<>()))
    .def("get_definitions_client",
      BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetDefinitionsClient,
      return_internal_reference<>()))
    .def("get_market_data_client",
      BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetMarketDataClient,
      return_internal_reference<>()))
    .def("get_charting_client",
      BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetChartingClient,
      return_internal_reference<>()))
    .def("get_compliance_client",
      BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetComplianceClient,
      return_internal_reference<>()))
    .def("get_order_execution_client",
      BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetOrderExecutionClient,
      return_internal_reference<>()))
    .def("get_risk_client", BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetRiskClient,
      return_internal_reference<>()))
    .def("get_time_client", BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetTimeClient,
      return_internal_reference<>()))
    .def("build_timer", ReleaseUniquePtr<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::BuildTimer))
    .def("open", BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::Open))
    .def("close", BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::Close));
}

void Nexus::Python::ExportServiceClients() {
  ExportVirtualServiceClients();
  ExportApplicationServiceClients();
  ExportTestEnvironment();
  ExportTestServiceClients();
  ExportTestTimeClient();
  ExportTestTimer();
}

void Nexus::Python::ExportTestEnvironment() {
  class_<TestEnvironment, std::shared_ptr<TestEnvironment>, boost::noncopyable>(
      "TestEnvironment", init<>())
    .def("set_time", BlockingFunction(&TestEnvironment::SetTime))
    .def("advance_time", BlockingFunction(&TestEnvironment::AdvanceTime))
    .def("update", BlockingFunction(
      static_cast<void (TestEnvironment::*)(const Security&, const BboQuote&)>(
      &TestEnvironment::Update)))
    .def("monitor_order_submissions", &TestEnvironmentMonitorOrderSubmissions)
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
    .def("get_service_locator_instance",
      &TestEnvironment::GetServiceLocatorInstance,
      return_internal_reference<>())
    .def("get_uid_instance", &TestEnvironment::GetUidInstance,
      return_internal_reference<>())
    .def("get_administration_instance",
      &TestEnvironment::GetAdministrationInstance,
      return_internal_reference<>())
    .def("get_market_data_instance", &TestEnvironment::GetMarketDataInstance,
      return_internal_reference<>())
    .def("get_order_execution_instance",
      &TestEnvironment::GetOrderExecutionInstance,
      return_internal_reference<>())
    .def("open", BlockingFunction(&TestEnvironment::Open))
    .def("close", BlockingFunction(&TestEnvironment::Close));
}

void Nexus::Python::ExportTestServiceClients() {
  class_<PythonTestServiceClients, boost::noncopyable,
      bases<VirtualServiceClients>>("TestServiceClients", no_init)
    .def("__init__", make_constructor(&BuildTestServiceClients))
    .def("get_service_locator_client",
      BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetServiceLocatorClient,
      return_internal_reference<>()))
    .def("get_registry_client", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetRegistryClient,
      return_internal_reference<>()))
    .def("get_administration_client",
      BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetAdministrationClient,
      return_internal_reference<>()))
    .def("get_definitions_client", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetDefinitionsClient,
      return_internal_reference<>()))
    .def("get_market_data_client", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetMarketDataClient,
      return_internal_reference<>()))
    .def("get_charting_client", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetChartingClient,
      return_internal_reference<>()))
    .def("get_compliance_client", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetComplianceClient,
      return_internal_reference<>()))
    .def("get_order_execution_client",
      BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetOrderExecutionClient,
      return_internal_reference<>()))
    .def("get_risk_client", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetRiskClient, return_internal_reference<>()))
    .def("get_time_client", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetTimeClient, return_internal_reference<>()))
    .def("build_timer", ReleaseUniquePtr<PythonTestServiceClients>(
      &PythonTestServiceClients::BuildTimer))
    .def("open", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::Open))
    .def("close", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::Close));
}

void Nexus::Python::ExportTestTimeClient() {
  class_<PythonTestTimeClient, boost::noncopyable, bases<VirtualTimeClient>>(
      "TestTimeClient", no_init)
    .def("__init__", make_constructor(&BuildTestTimeClient))
    .def("get_time", &PythonTestTimeClient::GetTime)
    .def("open", BlockingFunction<PythonTestTimeClient>(
      &PythonTestTimeClient::Open))
    .def("close", BlockingFunction<PythonTestTimeClient>(
      &PythonTestTimeClient::Close));
}

void Nexus::Python::ExportTestTimer() {
  class_<PythonTestTimer, boost::noncopyable, bases<VirtualTimer>>("TestTimer",
      no_init)
    .def("__init__", make_constructor(&BuildTestTimer))
    .def("start", BlockingFunction<PythonTestTimer>(&PythonTestTimer::Start))
    .def("cancel", BlockingFunction<PythonTestTimer>(&PythonTestTimer::Cancel))
    .def("wait", BlockingFunction<PythonTestTimer>(&PythonTestTimer::Wait));
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
