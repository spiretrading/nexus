#include "Spire/Canvas/Types/OrderImbalanceRecordType.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Spire/Canvas/Records/Record.hpp"
#include "Spire/Canvas/Types/DateTimeType.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/SecurityType.hpp"
#include "Spire/Canvas/Types/SideType.hpp"

using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  std::shared_ptr<RecordType> MakeType() {
    vector<RecordType::Field> fields;
    fields.emplace_back("security", SecurityType::GetInstance());
    fields.emplace_back("side", SideType::GetInstance());
    fields.emplace_back("size", IntegerType::GetInstance());
    fields.emplace_back("reference_price", MoneyType::GetInstance());
    fields.emplace_back("timestamp", DateTimeType::GetInstance());
    return MakeRecordType("Order Imbalance", std::move(fields));
  }
}

const RecordType& Spire::GetOrderImbalanceRecordType() {
  static auto type = MakeType();
  return *type;
}

Record OrderImbalanceToRecordConverter::operator ()(
    const SequencedOrderImbalance& value) const {
  return Record({ value->m_security, value->m_side, value->m_size,
    value->m_referencePrice, value->m_timestamp });
}
