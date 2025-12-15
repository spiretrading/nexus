#include "Spire/Canvas/Types/PortfolioEntryRecordType.hpp"
#include "Spire/Canvas/Records/Record.hpp"
#include "Spire/Canvas/Types/CurrencyType.hpp"
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
    fields.emplace_back("currency", CurrencyType::GetInstance());
    fields.emplace_back("side", SideType::GetInstance());
    fields.emplace_back("quantity", IntegerType::GetInstance());
    fields.emplace_back("cost_basis", MoneyType::GetInstance());
    fields.emplace_back("gross", MoneyType::GetInstance());
    fields.emplace_back("fees", MoneyType::GetInstance());
    fields.emplace_back("volume", IntegerType::GetInstance());
    fields.emplace_back("transactions", IntegerType::GetInstance());
    return MakeRecordType("Portfolio Entry", std::move(fields));
  }
}

const RecordType& Spire::GetPortfolioEntryRecordType() {
  static auto type = MakeType();
  return *type;
}

Record PortfolioEntryToRecordConverter::operator ()(
    const PortfolioUpdateEntry& value) const {
  return Record({value.m_security_inventory.m_position.m_security,
    value.m_security_inventory.m_position.m_currency,
    get_side(value.m_security_inventory.m_position),
    abs(value.m_security_inventory.m_position.m_quantity),
    value.m_security_inventory.m_position.m_cost_basis,
    value.m_security_inventory.m_gross_profit_and_loss,
    value.m_security_inventory.m_fees, value.m_security_inventory.m_volume,
    static_cast<Quantity>(value.m_security_inventory.m_transaction_count)});
}
