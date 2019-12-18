#include "Nexus/Python/RiskService.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/RiskService/RiskParameters.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Nexus;
using namespace Nexus::RiskService;
using namespace boost;
using namespace boost::posix_time;
using namespace pybind11;

void Nexus::Python::ExportRiskService(pybind11::module& module) {
  auto submodule = module.def_submodule("risk_service");
  ExportRiskParameters(submodule);
  ExportRiskState(submodule);
}

void Nexus::Python::ExportRiskParameters(pybind11::module& module) {
  class_<RiskParameters>(module, "RiskParameters")
    .def(init())
    .def(init<const RiskParameters&>())
    .def_readwrite("currency", &RiskParameters::m_currency)
    .def_readwrite("buying_power", &RiskParameters::m_buyingPower)
    .def_readwrite("allowed_state", &RiskParameters::m_allowedState)
    .def_readwrite("net_loss", &RiskParameters::m_netLoss)
    .def_readwrite("loss_from_top", &RiskParameters::m_lossFromTop)
    .def_readwrite("transition_time", &RiskParameters::m_transitionTime)
    .def(self == self)
    .def(self != self);
}

void Nexus::Python::ExportRiskState(pybind11::module& module) {
  auto outer = class_<RiskState>(module, "RiskState")
    .def(init())
    .def(init<RiskState::Type>())
    .def(init<RiskState::Type, ptime>())
    .def(init<const RiskState&>())
    .def_readwrite("type", &RiskState::m_type)
    .def_readwrite("expiry", &RiskState::m_expiry)
    .def(self == self)
    .def(self != self);
  enum_<RiskState::Type::Type>(outer, "Type")
    .value("NONE", RiskState::Type::NONE)
    .value("ACTIVE", RiskState::Type::ACTIVE)
    .value("CLOSE_ORDERS", RiskState::Type::CLOSE_ORDERS)
    .value("DISABLED", RiskState::Type::DISABLED);
}
