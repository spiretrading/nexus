#include "Spire/Canvas/Types/ExecutionReportRecordType.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Spire/Canvas/Records/Record.hpp"
#include "Spire/Canvas/Types/DateTimeType.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Canvas/Types/OrderStatusType.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/TextType.hpp"

using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace std;

namespace {
  std::shared_ptr<RecordType> MakeType() {
    std::vector<RecordType::Field> fields;
    fields.emplace_back("id", IntegerType::GetInstance());
    fields.emplace_back("sequence", IntegerType::GetInstance());
    fields.emplace_back("timestamp", DateTimeType::GetInstance());
    fields.emplace_back("status", OrderStatusType::GetInstance());
    fields.emplace_back("last_quantity", IntegerType::GetInstance());
    fields.emplace_back("last_price", MoneyType::GetInstance());
    fields.emplace_back("liquidity_flag", TextType::GetInstance());
    fields.emplace_back("last_market", TextType::GetInstance());
    fields.emplace_back("execution_fee", MoneyType::GetInstance());
    fields.emplace_back("processing_fee", MoneyType::GetInstance());
    fields.emplace_back("commission", MoneyType::GetInstance());
    fields.emplace_back("text", TextType::GetInstance());
    return MakeRecordType("Execution Report", std::move(fields));
  }
}

const RecordType& Spire::GetExecutionReportRecordType() {
  static auto type = MakeType();
  return *type;
}

Record ExecutionReportToRecordConverter::operator ()(
    const ExecutionReport& value) const {
  return Record({ static_cast<Quantity>(value.m_id),
    static_cast<Quantity>(value.m_sequence), value.m_timestamp, value.m_status,
    value.m_lastQuantity, value.m_lastPrice, value.m_liquidityFlag,
    value.m_lastMarket, value.m_executionFee, value.m_processingFee,
    value.m_commission, value.m_text });
}
