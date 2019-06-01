#include "Nexus/Python/RiskService.hpp"
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/Copy.hpp>
#include <Beam/Python/Enum.hpp>
#include "Nexus/RiskService/RiskParameters.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Queries;
using namespace Nexus;
using namespace Nexus::RiskService;
using namespace boost;
using namespace boost::posix_time;
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
  ExportRiskState();
}

void Nexus::Python::ExportRiskParameters() {
  class_<RiskParameters>("RiskParameters", init<>())
    .def("__copy__", &MakeCopy<RiskParameters>)
    .def("__deepcopy__", &MakeDeepCopy<RiskParameters>)
    .def_readwrite("currency", &RiskParameters::m_currency)
    .def_readwrite("buying_power", &RiskParameters::m_buyingPower)
    .def_readwrite("allowed_state", &RiskParameters::m_allowedState)
    .def_readwrite("net_loss", &RiskParameters::m_netLoss)
    .def_readwrite("loss_from_top", &RiskParameters::m_lossFromTop)
    .add_property("transition_time",
      make_getter(&RiskParameters::m_transitionTime,
      return_value_policy<return_by_value>()),
      make_setter(&RiskParameters::m_transitionTime,
      return_value_policy<return_by_value>()))
    .def(self == self)
    .def(self != self);
}

void Nexus::Python::ExportRiskState() {
  {
    auto outerScope = scope(class_<RiskState>("RiskState", init<>())
      .def(init<RiskState::Type>())
      .def(init<RiskState::Type, ptime>())
      .def("__copy__", &MakeCopy<RiskState>)
      .def("__deepcopy__", &MakeDeepCopy<RiskState>)
      .add_property("type", make_getter(&RiskState::m_type,
        return_value_policy<return_by_value>()), make_setter(&RiskState::m_type,
        return_value_policy<return_by_value>()))
      .add_property("expiry", make_getter(&RiskState::m_expiry,
        return_value_policy<return_by_value>()),
        make_setter(&RiskState::m_expiry,
        return_value_policy<return_by_value>()))
      .def(self == self)
      .def(self != self));
    enum_<RiskState::Type::Type>("Type")
      .value("NONE", RiskState::Type::NONE)
      .value("ACTIVE", RiskState::Type::ACTIVE)
      .value("CLOSE_ORDERS", RiskState::Type::CLOSE_ORDERS)
      .value("DISABLED", RiskState::Type::DISABLED);
  }
  ExportEnum<RiskState::Type>();
}
