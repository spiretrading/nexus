#include "Spire/Canvas/Types/OrderFieldsRecordType.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Spire/Canvas/Records/Record.hpp"
#include "Spire/Canvas/Types/CurrencyType.hpp"
#include "Spire/Canvas/Types/DestinationType.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Canvas/Types/OrderTypeType.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/SideType.hpp"
#include "Spire/Canvas/Types/TickerType.hpp"
#include "Spire/Canvas/Types/TimeInForceType.hpp"

using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  std::shared_ptr<RecordType> MakeType() {
    vector<RecordType::Field> fields;
    fields.emplace_back("ticker", TickerType::GetInstance());
    fields.emplace_back("currency", CurrencyType::GetInstance());
    fields.emplace_back("type", OrderTypeType::GetInstance());
    fields.emplace_back("side", SideType::GetInstance());
    fields.emplace_back("destination", DestinationType::GetInstance());
    fields.emplace_back("quantity", IntegerType::GetInstance());
    fields.emplace_back("price", MoneyType::GetInstance());
    fields.emplace_back("time_in_force", TimeInForceType::GetInstance());
    return MakeRecordType("Order Fields", std::move(fields));
  }
}

const RecordType& Spire::GetOrderFieldsRecordType() {
  static auto type = MakeType();
  return *type;
}

Record OrderFieldsToRecordConverter::operator ()(
    const OrderFields& value) const {
  return Record({value.m_ticker, value.m_currency, value.m_type, value.m_side,
    value.m_destination, value.m_quantity, value.m_price,
    value.m_time_in_force});
}
