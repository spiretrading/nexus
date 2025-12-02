#include "Nexus/Python/Nexus.hpp"
#include <pybind11/pybind11.h>

using namespace Nexus;
using namespace Nexus::Python;
using namespace pybind11;

PYBIND11_MODULE(nexus, m) {
  auto beam_module = module::import("beam");
  export_queries(m);
  export_accounting(m);
  export_administration_service(m);
  export_charting_service(m);
  export_compliance(m);
  export_definitions(m);
  export_definitions_service(m);
  export_fee_handling(m);
  export_market_data_service(m);
  export_order_execution_service(m);
  export_risk_service(m);
  export_service_clients(m);
  export_backtester(m);
  export_test_environment(m);
  export_technical_analysis(m);
}
