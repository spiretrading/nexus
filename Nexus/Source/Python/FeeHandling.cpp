#include "Nexus/Python/FeeHandling.hpp"
#include <Beam/Python/BoostPython.hpp>
#include "Nexus/FeeHandling/PureFeeTable.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace Nexus;
using namespace Nexus::Python;
using namespace std;

void Nexus::Python::ExportFeeHandling() {
  ExportPureFeeTable();
}

void Nexus::Python::ExportPureFeeTable() {
  class_<PureFeeTable>("PureFeeTable");
  def("parse_pure_fee_table", &ParsePureFeeTable);
  def("calculate_fee", &CalculateFee);
}
