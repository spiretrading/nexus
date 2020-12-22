#include "Spire/Canvas/Types/QuoteRecordType.hpp"
#include "Nexus/Definitions/Quote.hpp"
#include "Spire/Canvas/Records/Record.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/SideType.hpp"

using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  std::shared_ptr<RecordType> MakeType() {
    vector<RecordType::Field> fields;
    fields.emplace_back("price", MoneyType::GetInstance());
    fields.emplace_back("side", SideType::GetInstance());
    fields.emplace_back("quantity", IntegerType::GetInstance());
    return MakeRecordType("Quote", std::move(fields));
  }
}

const RecordType& Spire::GetQuoteRecordType() {
  static auto type = MakeType();
  return *type;
}

Record QuoteToRecordConverter::operator ()(const Quote& value) const {
  return Record({ value.m_price, value.m_side, value.m_size });
}
