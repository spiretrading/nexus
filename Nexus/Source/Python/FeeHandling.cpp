#include "Nexus/Python/FeeHandling.hpp"
#include <Beam/Python/BoostPython.hpp>
#include "Nexus/FeeHandling/AsxtFeeTable.hpp"
#include "Nexus/FeeHandling/PureFeeTable.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace std;

void Nexus::Python::ExportAsxtFeeTable() {
  class_<AsxtFeeTable>("AsxtFeeTable", init<>())
    .def_readwrite("spire_fee", &AsxtFeeTable::m_spireFee)
    .def_readwrite("clearing_rate", &AsxtFeeTable::m_clearingRate)
    .def_readwrite("trade_rate", &AsxtFeeTable::m_tradeRate)
    .def_readwrite("gst_rate", &AsxtFeeTable::m_gstRate)
    .def_readwrite("trade_fee_cap", &AsxtFeeTable::m_tradeFeeCap);
  def("parse_asx_fee_table", &ParseAsxFeeTable);
  def("calculate_fee", static_cast<ExecutionReport (*)(const AsxtFeeTable&,
    const ExecutionReport&)>(&CalculateFee));
}

void Nexus::Python::ExportFeeHandling() {
  ExportAsxtFeeTable();
  ExportPureFeeTable();
}

void Nexus::Python::ExportPureFeeTable() {
  class_<PureFeeTable>("PureFeeTable");
  def("parse_pure_fee_table", &ParsePureFeeTable);
  def("calculate_fee", static_cast<Money (*)(const PureFeeTable&,
    const Security&, const ExecutionReport&)>(&CalculateFee));
}
