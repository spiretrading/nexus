#include "Spire/Canvas/Types/PortfolioEntryRecordType.hpp"
#include "Spire/Canvas/Records/Record.hpp"
#include "Spire/Canvas/Types/CurrencyType.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/SecurityType.hpp"
#include "Spire/Canvas/Types/SideType.hpp"

using namespace Nexus;
using namespace Nexus::Accounting;
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
    const SpirePortfolio::UpdateEntry& value) const {
  return Record( {
    value.m_securityInventory.m_position.m_key.m_index,
    value.m_securityInventory.m_position.m_key.m_currency,
    GetSide(value.m_securityInventory.m_position),
    Abs(value.m_securityInventory.m_position.m_quantity),
    value.m_securityInventory.m_position.m_costBasis,
    value.m_securityInventory.m_grossProfitAndLoss,
    value.m_securityInventory.m_fees,
    value.m_securityInventory.m_volume,
    static_cast<Quantity>(value.m_securityInventory.m_transactionCount) });
}
