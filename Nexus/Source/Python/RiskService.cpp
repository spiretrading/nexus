#include "Nexus/Python/RiskService.hpp"
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/Queries.hpp>
#include "Nexus/RiskService/RiskParameters.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Queries;
using namespace Nexus;
using namespace Nexus::RiskService;
using namespace boost;
using namespace boost::python;
using namespace std;

void Nexus::Python::ExportRiskService() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".risk_service");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("risk_service") = nestedModule;
  scope parent = nestedModule;
  ExportRiskParameters();
}

void Nexus::Python::ExportRiskParameters() {
  class_<RiskParameters>("RiskParameters", init<>())
    .def_readwrite("currency", &RiskParameters::m_currency)
    .def_readwrite("buying_power", &RiskParameters::m_buyingPower)
    .def_readwrite("allowed_state", &RiskParameters::m_allowedState)
    .def_readwrite("net_loss", &RiskParameters::m_netLoss)
    .def_readwrite("loss_from_top", &RiskParameters::m_lossFromTop)
    .def_readwrite("transition_time", &RiskParameters::m_transitionTime)
    .def(self == self)
    .def(self != self);
}
