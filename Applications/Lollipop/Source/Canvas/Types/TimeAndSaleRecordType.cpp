#include "Spire/Canvas/Types/TimeAndSaleRecordType.hpp"
#include "Spire/Canvas/Records/Record.hpp"
#include "Spire/Canvas/Types/DateTimeType.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/TextType.hpp"

using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  std::shared_ptr<RecordType> MakeType() {
    vector<RecordType::Field> fields;
    fields.emplace_back("timestamp", DateTimeType::GetInstance());
    fields.emplace_back("price", MoneyType::GetInstance());
    fields.emplace_back("size", IntegerType::GetInstance());
    fields.emplace_back("condition", TextType::GetInstance());
    fields.emplace_back("market_center", TextType::GetInstance());
    return MakeRecordType("Time and Sale", std::move(fields));
  }
}

const RecordType& Spire::GetTimeAndSaleRecordType() {
  static auto type = MakeType();
  return *type;
}

Record TimeAndSaleToRecordConverter::operator ()(
    const SequencedTimeAndSale& value) const {
  return Record({ value->m_timestamp, value->m_price, value->m_size,
    value->m_condition.m_code, value->m_marketCenter });
}
