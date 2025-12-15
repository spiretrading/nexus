#include "Spire/Catalog/BuiltInCatalogEntry.hpp"
#include <Beam/Utilities/NotSupportedException.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "Spire/Canvas/Common/CustomNode.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/ControlNodes/AggregateNode.hpp"
#include "Spire/Canvas/ControlNodes/ChainNode.hpp"
#include "Spire/Canvas/ControlNodes/SpawnNode.hpp"
#include "Spire/Canvas/ControlNodes/UntilNode.hpp"
#include "Spire/Canvas/ControlNodes/WhenNode.hpp"
#include "Spire/Canvas/IONodes/FilePathNode.hpp"
#include "Spire/Canvas/IONodes/FileReaderNode.hpp"
#include "Spire/Canvas/LuaNodes/LuaScriptNode.hpp"
#include "Spire/Canvas/MarketDataNodes/BboQuoteQueryNode.hpp"
#include "Spire/Canvas/MarketDataNodes/OrderImbalanceQueryNode.hpp"
#include "Spire/Canvas/MarketDataNodes/TimeAndSaleQueryNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/OrderExecutionNodes/DefaultCurrencyNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/ExecutionReportMonitorNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderTaskNodes.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderWrapperTaskNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SecurityPortfolioNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/Records/QueryNode.hpp"
#include "Spire/Canvas/Records/RecordNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ProxyNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/StandardNodes/AbsNode.hpp"
#include "Spire/Canvas/StandardNodes/AdditionNode.hpp"
#include "Spire/Canvas/StandardNodes/AlarmNode.hpp"
#include "Spire/Canvas/StandardNodes/CeilNode.hpp"
#include "Spire/Canvas/StandardNodes/CountNode.hpp"
#include "Spire/Canvas/StandardNodes/CurrentDateNode.hpp"
#include "Spire/Canvas/StandardNodes/CurrentDateTimeNode.hpp"
#include "Spire/Canvas/StandardNodes/CurrentTimeNode.hpp"
#include "Spire/Canvas/StandardNodes/DistinctNode.hpp"
#include "Spire/Canvas/StandardNodes/DivisionNode.hpp"
#include "Spire/Canvas/StandardNodes/EqualsNode.hpp"
#include "Spire/Canvas/StandardNodes/FilterNode.hpp"
#include "Spire/Canvas/StandardNodes/FirstNode.hpp"
#include "Spire/Canvas/StandardNodes/FloorNode.hpp"
#include "Spire/Canvas/StandardNodes/FoldNode.hpp"
#include "Spire/Canvas/StandardNodes/GreaterNode.hpp"
#include "Spire/Canvas/StandardNodes/GreaterOrEqualsNode.hpp"
#include "Spire/Canvas/StandardNodes/IfNode.hpp"
#include "Spire/Canvas/StandardNodes/LastNode.hpp"
#include "Spire/Canvas/StandardNodes/LesserNode.hpp"
#include "Spire/Canvas/StandardNodes/LesserOrEqualsNode.hpp"
#include "Spire/Canvas/StandardNodes/MaxNode.hpp"
#include "Spire/Canvas/StandardNodes/MinNode.hpp"
#include "Spire/Canvas/StandardNodes/MultiplicationNode.hpp"
#include "Spire/Canvas/StandardNodes/NotNode.hpp"
#include "Spire/Canvas/StandardNodes/PreviousNode.hpp"
#include "Spire/Canvas/StandardNodes/RangeNode.hpp"
#include "Spire/Canvas/StandardNodes/RoundNode.hpp"
#include "Spire/Canvas/StandardNodes/SubtractionNode.hpp"
#include "Spire/Canvas/StandardNodes/TimeRangeParameterNode.hpp"
#include "Spire/Canvas/StandardNodes/TimerNode.hpp"
#include "Spire/Canvas/StandardNodes/UnequalNode.hpp"
#include "Spire/Canvas/SystemNodes/BlotterTaskMonitorNode.hpp"
#include "Spire/Canvas/SystemNodes/InteractionsNode.hpp"
#include "Spire/Canvas/Types/ExecutionReportRecordType.hpp"
#include "Spire/Canvas/Types/OrderFieldsRecordType.hpp"
#include "Spire/Canvas/Types/PortfolioEntryRecordType.hpp"
#include "Spire/Canvas/Types/QuoteRecordType.hpp"
#include "Spire/Canvas/ValueNodes/BooleanNode.hpp"
#include "Spire/Canvas/ValueNodes/CurrencyNode.hpp"
#include "Spire/Canvas/ValueNodes/DateTimeNode.hpp"
#include "Spire/Canvas/ValueNodes/DecimalNode.hpp"
#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/ValueNodes/DurationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderStatusNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderTypeNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TextNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeRangeNode.hpp"
#include "Spire/Canvas/ValueNodes/VenueNode.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::uuids;
using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  string_generator stringUuidGenerator;
}

const uuid BuiltInCatalogEntry::ABS_UUID = stringUuidGenerator(
  "{77a02a82-cee1-4dba-9bb1-5e7dbb76287a}");
const uuid BuiltInCatalogEntry::ADDITION_UUID = stringUuidGenerator(
  "{f3a1ca9e-52eb-49f0-b0a3-4a7930b6e375}");
const uuid BuiltInCatalogEntry::AGGREGATE_TASK_UUID =
  stringUuidGenerator("{60071336-beb4-4c39-9889-5980dcdfe6d2}");
const uuid BuiltInCatalogEntry::ALARM_UUID = stringUuidGenerator(
  "{5e93981b-6d91-49e9-8c67-c28f2848e297}");
const uuid BuiltInCatalogEntry::ASK_ORDER_TASK_UUID = stringUuidGenerator(
  "{cd1091ac-d8be-47c0-830d-2521edde18af}");
const uuid BuiltInCatalogEntry::BBO_QUOTE_QUERY_UUID = stringUuidGenerator(
  "{5337c510-7c4f-11e1-b0c4-0800200c9a66}");
const uuid BuiltInCatalogEntry::BID_ORDER_TASK_UUID = stringUuidGenerator(
  "{96c62c40-1cae-4c9e-8824-f1ca989a4e25}");
const uuid BuiltInCatalogEntry::BOOLEAN_UUID = stringUuidGenerator(
  "{f6cf07c7-28c7-4643-b16d-1b7ed9981251}");
const uuid BuiltInCatalogEntry::CEIL_UUID = stringUuidGenerator(
  "{8abe970b-8673-4112-a565-a348233f2f88}");
const uuid BuiltInCatalogEntry::CHAIN_UUID = stringUuidGenerator(
  "{8b557053-8eaf-4fc8-b549-e06d4c5380a2}");
const uuid BuiltInCatalogEntry::COUNT_UUID = stringUuidGenerator(
  "{91c14acf-b15e-4056-aed6-f8959cd000ce}");
const uuid BuiltInCatalogEntry::CURRENCY_UUID = stringUuidGenerator(
  "{a21f3f70-3db8-42f6-bc90-5915e978a20f}");
const uuid BuiltInCatalogEntry::CURRENT_DATE_UUID = stringUuidGenerator(
  "{bcd74060-ed30-460f-87d7-de9fe3566d98}");
const uuid BuiltInCatalogEntry::CURRENT_DATE_TIME_UUID = stringUuidGenerator(
  "{9a0f516e-b50e-443a-8cb2-5b060daa6ca6}");
const uuid BuiltInCatalogEntry::CURRENT_TIME_UUID = stringUuidGenerator(
  "{287e6ff8-958c-4638-bb99-bc88753b8840}");
const uuid BuiltInCatalogEntry::DATE_TIME_UUID = stringUuidGenerator(
  "{0927666f-cc81-4685-8d18-349c48fde75c}");
const uuid BuiltInCatalogEntry::DECIMAL_UUID = stringUuidGenerator(
  "{215aa16a-a807-4ff4-bcce-1ada4da62a5f}");
const uuid BuiltInCatalogEntry::DEFAULT_CURRENCY_UUID = stringUuidGenerator(
  "{ca3d8ed5-83cd-4401-ad21-5871d8aa3f64}");
const uuid BuiltInCatalogEntry::DESTINATION_UUID = stringUuidGenerator(
  "{57fe8798-49ad-48ce-b8a7-e61d38abc542}");
const uuid BuiltInCatalogEntry::DISTINCT_UUID = stringUuidGenerator(
  "{2308791f-2b74-4730-a281-b594e16400b4}");
const uuid BuiltInCatalogEntry::DIVISION_UUID = stringUuidGenerator(
  "{f34c2004-02eb-4aec-871a-5d852e0659f7}");
const uuid BuiltInCatalogEntry::DURATION_UUID = stringUuidGenerator(
  "{156def10-5dee-11df-a08a-0800200c9a66}");
const uuid BuiltInCatalogEntry::EQUAL_UUID = stringUuidGenerator(
  "{d45c5e78-8f80-45ce-85fa-e55c75fdfbdd}");
const uuid BuiltInCatalogEntry::EXECUTION_REPORT_MONITOR_UUID =
  stringUuidGenerator("{f4d11994-7b82-4cb2-96a3-183e454eec9e}");
const uuid BuiltInCatalogEntry::EXECUTION_REPORT_RECORD_UUID =
  stringUuidGenerator("{4536790d-4e9c-4840-ad71-ba09fe27922c}");
const uuid BuiltInCatalogEntry::FILE_PATH_UUID = stringUuidGenerator(
  "{5eb93b24-f11e-41f0-be50-f7b1a731a0d8}");
const uuid BuiltInCatalogEntry::FILE_READER_UUID = stringUuidGenerator(
  "{352d9796-5912-4f7d-8328-5eabac3fc72c}");
const uuid BuiltInCatalogEntry::FILTER_UUID = stringUuidGenerator(
  "{648bea47-aff2-48f6-bb45-3cecaacc0aac}");
const uuid BuiltInCatalogEntry::FIRST_UUID = stringUuidGenerator(
  "{f26b1964-66d8-41cc-9399-921cfb8a3818}");
const uuid BuiltInCatalogEntry::FLOOR_UUID = stringUuidGenerator(
  "{e2b8a69b-2412-4a18-b945-68d2cf291407}");
const uuid BuiltInCatalogEntry::FOLD_UUID = stringUuidGenerator(
  "{29738856-dcb8-45fe-9e84-d9bd2946bdeb}");
const uuid BuiltInCatalogEntry::GREATER_EQUAL_UUID = stringUuidGenerator(
  "{4bd57eff-774f-4e2c-a127-4194a5a57434}");
const uuid BuiltInCatalogEntry::GREATER_UUID = stringUuidGenerator(
  "{b9285f5f-d02b-44d1-b8f9-85f4521355d1}");
const uuid BuiltInCatalogEntry::IF_UUID = stringUuidGenerator(
  "{619bad05-1fe3-4482-aa96-56e3cf895276}");
const uuid BuiltInCatalogEntry::INTEGER_UUID = stringUuidGenerator(
  "{0ff0640f-e00d-4a39-81e1-2efb64721681}");
const uuid BuiltInCatalogEntry::IS_TERMINAL_UUID = stringUuidGenerator(
  "{556b1d84-0d33-40e0-988b-0db86336c065}");
const uuid BuiltInCatalogEntry::LAST_UUID = stringUuidGenerator(
  "{23a93e8c-7c3b-4fcf-97fa-700e7345d428}");
const uuid BuiltInCatalogEntry::LESS_EQUAL_UUID = stringUuidGenerator(
  "{c32348eb-a25d-4c07-abd4-31a211f1463d}");
const uuid BuiltInCatalogEntry::LESS_UUID = stringUuidGenerator(
  "{881edd71-990d-4522-af08-061135085a70}");
const uuid BuiltInCatalogEntry::LIMIT_ASK_ORDER_TASK_UUID =
  stringUuidGenerator("{e1cc46b6-3295-4026-88cc-1f95116fb6b5}");
const uuid BuiltInCatalogEntry::LIMIT_BID_ORDER_TASK_UUID =
  stringUuidGenerator("{91517c38-2106-4ff3-ac0a-e664cb3724a0}");
const uuid BuiltInCatalogEntry::LIMIT_ORDER_TASK_UUID = stringUuidGenerator(
  "{f6edf92f-ca10-4ef2-ac6c-27e733c91476}");
const uuid BuiltInCatalogEntry::LUA_SCRIPT_UUID = stringUuidGenerator(
  "{5fe6f1d4-6ef9-4209-bb8e-4830ecc96720}");
const uuid BuiltInCatalogEntry::MARKET_ASK_ORDER_TASK_UUID =
  stringUuidGenerator("{d7212af0-9166-48df-9ea2-61a530f9e533}");
const uuid BuiltInCatalogEntry::MARKET_BID_ORDER_TASK_UUID =
  stringUuidGenerator("{cebd25b9-1bcc-4613-8a17-207bd4c6193a}");
const uuid BuiltInCatalogEntry::MARKET_ORDER_TASK_UUID = stringUuidGenerator(
  "{6bd96fe8-10b7-49ae-9ffc-32b4b34818f2}");
const uuid BuiltInCatalogEntry::MARKET_UUID = stringUuidGenerator(
  "{7c59c5ef-25aa-454d-89ac-27e9013a1e2d}");
const uuid BuiltInCatalogEntry::MAX_FLOOR_UUID = stringUuidGenerator(
  "{63a4bba0-d2da-11e2-8b8b-0800200c9a66}");
const uuid BuiltInCatalogEntry::MAX_UUID = stringUuidGenerator(
  "{bbca92c6-2c8c-4076-a3db-dbb094d92436}");
const uuid BuiltInCatalogEntry::MIN_UUID = stringUuidGenerator(
  "{84cc6a5d-ee49-4b13-89ee-60b03bbd16bc}");
const uuid BuiltInCatalogEntry::MONEY_UUID = stringUuidGenerator(
  "{46a723f9-6263-4239-b389-6cded9596739}");
const uuid BuiltInCatalogEntry::MULTIPLICATION_UUID = stringUuidGenerator(
  "{ef66f68b-8150-486d-aef9-4b5c28c56aec}");
const uuid BuiltInCatalogEntry::MARKET_ORDER_IMBALANCE_UUID =
  stringUuidGenerator("{140370d0-edcb-11e2-91e2-0800200c9a66}");
const uuid BuiltInCatalogEntry::NONE_UUID = stringUuidGenerator(
  "{e1ad68d0-6a58-11e1-b0c4-0800200c9a66}");
const uuid BuiltInCatalogEntry::NOT_EQUAL_UUID = stringUuidGenerator(
  "{a274b335-69f8-4a8a-9b29-e224a4a6fd65}");
const uuid BuiltInCatalogEntry::NOT_UUID = stringUuidGenerator(
  "{324d0e49-d9fa-4b38-b5af-f0e4701b0414}");
const uuid BuiltInCatalogEntry::OPTIONAL_PRICE_UUID = stringUuidGenerator(
  "{945b1f50-78cf-11e3-981f-0800200c9a66}");
const uuid BuiltInCatalogEntry::ORDER_FIELDS_RECORD_UUID = stringUuidGenerator(
  "{b12d37d5-fbb3-48f0-95a1-19cce1343ac7}");
const uuid BuiltInCatalogEntry::ORDER_STATUS_UUID = stringUuidGenerator(
  "{1fda3a22-74ee-4ac3-bf62-51375082bb53}");
const uuid BuiltInCatalogEntry::ORDER_TYPE_UUID = stringUuidGenerator(
  "{13b1bf30-6efe-11e1-b0c4-0800200c9a66}");
const uuid BuiltInCatalogEntry::PORTFOLIO_ENTRY_RECORD_UUID =
  stringUuidGenerator("{01d81d17-e13c-4af2-a62d-8d0fa909c801}");
const uuid BuiltInCatalogEntry::PREVIOUS_UUID = stringUuidGenerator(
  "{528a357e-9997-4abf-8bfe-e981110badbd}");
const uuid BuiltInCatalogEntry::QUERY_UUID = stringUuidGenerator(
  "{42217a60-7ce9-11e1-b0c4-0800200c9a66}");
const uuid BuiltInCatalogEntry::QUOTE_RECORD_UUID = stringUuidGenerator(
  "{e83e566f-887a-4d13-ac3c-e57882053f56}");
const uuid BuiltInCatalogEntry::RANGE_UUID = stringUuidGenerator(
  "{2cfa0070-bd4c-11df-851a-0800200c9a66}");
const uuid BuiltInCatalogEntry::REFERENCE_UUID = stringUuidGenerator(
  "{ea17b970-72a1-11e1-b0c4-0800200c9a66}");
const uuid BuiltInCatalogEntry::ROUND_UUID = stringUuidGenerator(
  "{3e618b36-d131-4526-be01-69c98dccd6bc}");
const uuid BuiltInCatalogEntry::SECURITY_UUID = stringUuidGenerator(
  "{efb1c80d-9052-46b0-919e-37495602e0db}");
const uuid BuiltInCatalogEntry::SECURITY_PORTFOLIO_UUID = stringUuidGenerator(
  "{f89ef270-b30c-11df-94e2-0800200c9a66}");
const uuid BuiltInCatalogEntry::SIDE_UUID = stringUuidGenerator(
  "{fbd040ea-d3b4-4245-8caa-edf6a273a94e}");
const uuid BuiltInCatalogEntry::SINGLE_ORDER_TASK_UUID = stringUuidGenerator(
  "{ed2824b0-6ee6-11e1-b0c4-0800200c9a66}");
const uuid BuiltInCatalogEntry::SPAWN_TASK_UUID = stringUuidGenerator(
  "{dcf7a430-4bf2-11df-9879-0800200c9a66}");
const uuid BuiltInCatalogEntry::SUBTRACTION_UUID = stringUuidGenerator(
  "{b0f0b4ac-f6d6-4135-b03c-e35e2ea0b231}");
const uuid BuiltInCatalogEntry::TASK_STATE_UUID = stringUuidGenerator(
  "{abde5c5c-b388-4fe2-a451-93f0715277ed}");
const uuid BuiltInCatalogEntry::TASK_STATE_MONITOR_UUID = stringUuidGenerator(
  "{265fbaf3-ad85-40e3-a39f-f6bacc3b26de}");
const uuid BuiltInCatalogEntry::TEXT_UUID = stringUuidGenerator(
  "{713dcb76-8450-4e25-ad22-51de1ba54c6a}");
const uuid BuiltInCatalogEntry::TIME_AND_SALE_QUERY_UUID = stringUuidGenerator(
  "{c9ff5de0-f838-11e3-a3ac-0800200c9a66}");
const uuid BuiltInCatalogEntry::TIME_IN_FORCE_UUID = stringUuidGenerator(
  "{dbb7331f-4d69-4112-abec-cf3af9f58060}");
const uuid BuiltInCatalogEntry::TIME_RANGE_PARAMETER_UUID = stringUuidGenerator(
  "{4f0c1c80-ed95-11e2-91e2-0800200c9a66}");
const uuid BuiltInCatalogEntry::TIMER_UUID = stringUuidGenerator(
  "{7457b518-984e-4ddb-b02d-068d2589e8c0}");
const uuid BuiltInCatalogEntry::UNTIL_TASK_UUID = stringUuidGenerator(
  "{bcb18a51-02af-4533-b0d1-d05230c3c041}");
const uuid BuiltInCatalogEntry::WHEN_TASK_UUID = stringUuidGenerator(
  "{938ffdaa-aea2-4f07-bc6b-b6c44fb1f1e9}");

vector<unique_ptr<CatalogEntry>> BuiltInCatalogEntry::
    LoadBuiltInCatalogEntries() {
  vector<unique_ptr<CatalogEntry>> entries;
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(ABS_UUID,
    ":/icons/calculator.png", "", AbsNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(ADDITION_UUID,
    ":/icons/calculator.png", "", AdditionNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(AGGREGATE_TASK_UUID,
    ":/icons/flash_yellow.png", "", AggregateNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(ALARM_UUID,
    ":/icons/hourglass.png", "", AlarmNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(ASK_ORDER_TASK_UUID,
    ":/icons/flash_yellow.png", "", *GetAskOrderTaskNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(BBO_QUOTE_QUERY_UUID,
    ":/icons/chart_line.png", "", BboQuoteQueryNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(BID_ORDER_TASK_UUID,
    ":/icons/flash_yellow.png", "", *GetBidOrderTaskNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Boolean",
    BOOLEAN_UUID, ":/icons/logic_and.png", "", BooleanNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(CEIL_UUID,
    ":/icons/chart_line.png", "", CeilNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(CHAIN_UUID,
    ":/icons/link.png", "", ChainNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(
    COUNT_UUID, ":/icons/calculator.png", "", CountNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Currency",
    CURRENCY_UUID, ":/icons/currency_euro.png", "", CurrencyNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(CURRENT_DATE_UUID,
    ":/icons/hourglass.png", "", CurrentDateNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(
    CURRENT_DATE_TIME_UUID, ":/icons/hourglass.png", "",
    CurrentDateTimeNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(CURRENT_TIME_UUID,
    ":/icons/hourglass.png", "", CurrentTimeNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Date/Time",
    DATE_TIME_UUID, ":/icons/date_time.png", "", DateTimeNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Decimal",
    DECIMAL_UUID, ":/icons/weight.png", "", DecimalNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(DEFAULT_CURRENCY_UUID,
    ":/icons/currency_euro.png", "", DefaultCurrencyNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Destination",
    DESTINATION_UUID, ":/icons/target2.png", "", DestinationNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(
    DISTINCT_UUID, ":/icons/calculator.png", "", DistinctNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(DIVISION_UUID,
    ":/icons/calculator.png", "", DivisionNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Duration",
    DURATION_UUID, ":/icons/date_time.png", "", DurationNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(EQUAL_UUID,
    ":/icons/scales.png", "", EqualsNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(
    EXECUTION_REPORT_MONITOR_UUID, ":/icons/chart_line.png", "",
    ExecutionReportMonitorNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(
    EXECUTION_REPORT_RECORD_UUID, ":/icons/chart_line.png", "",
    RecordNode(GetExecutionReportRecordType())));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(FILE_PATH_UUID,
    ":/icons/floppy_disk.png", "", FilePathNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(FILE_READER_UUID,
    ":/icons/floppy_disk.png", "", FileReaderNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(FILTER_UUID,
    ":/icons/funnel.png", "", FilterNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(FIRST_UUID,
    ":/icons/note_pinned.png", "", FirstNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(FLOOR_UUID,
    ":/icons/calculator.png", "", FloorNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(FOLD_UUID,
    ":/icons/bookmark_blue.png", "", FoldNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(GREATER_EQUAL_UUID,
    ":/icons/scales.png", "", GreaterOrEqualsNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(GREATER_UUID,
    ":/icons/scales.png", "", GreaterNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(IF_UUID,
    ":/icons/symbol_questionmark.png", "", IfNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Integer",
    INTEGER_UUID, ":/icons/weight.png", "", IntegerNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(LAST_UUID,
    ":/icons/note_pinned.png", "", LastNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(LESS_EQUAL_UUID,
    ":/icons/scales.png", "", LesserOrEqualsNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(LESS_UUID,
    ":/icons/scales.png", "", LesserNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(
    LIMIT_ASK_ORDER_TASK_UUID, ":/icons/flash_yellow.png", "",
    *GetLimitAskOrderTaskNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(
    LIMIT_BID_ORDER_TASK_UUID, ":/icons/flash_yellow.png", "",
    *GetLimitBidOrderTaskNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(LIMIT_ORDER_TASK_UUID,
    ":/icons/flash_yellow.png", "", *GetLimitOrderTaskNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Lua Script",
    LUA_SCRIPT_UUID, ":/icons/bookmark_blue.png", "", LuaScriptNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(
    MARKET_ASK_ORDER_TASK_UUID, ":/icons/flash_yellow.png", "",
    *GetMarketAskOrderTaskNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(
    MARKET_BID_ORDER_TASK_UUID, ":/icons/flash_yellow.png", "",
    *GetMarketBidOrderTaskNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(
    MARKET_ORDER_TASK_UUID, ":/icons/flash_yellow.png", "",
    *GetMarketOrderTaskNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Max Floor",
    MAX_FLOOR_UUID, ":/icons/weight.png", "", MaxFloorNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(MAX_UUID,
    ":/icons/calculator.png", "", MaxNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(MIN_UUID,
    ":/icons/calculator.png", "", MinNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Money", MONEY_UUID,
    ":/icons/money.png", "", MoneyNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(MULTIPLICATION_UUID,
    ":/icons/calculator.png", "", MultiplicationNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(
    MARKET_ORDER_IMBALANCE_UUID, ":/icons/chart_line.png", "",
    OrderImbalanceQueryNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(NONE_UUID,
    ":/icons/flash_yellow.png", "", NoneNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(NOT_EQUAL_UUID,
    ":/icons/scales.png", "", UnequalNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(NOT_UUID,
    ":/icons/logic_not.png", "", NotNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Optional Price Node",
    OPTIONAL_PRICE_UUID, ":/icons/money.png", "", OptionalPriceNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(
    ORDER_FIELDS_RECORD_UUID, ":/icons/chart_line.png", "",
    RecordNode(GetOrderFieldsRecordType())));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Order Status",
    ORDER_STATUS_UUID, ":/icons/trafficlight_green.png", "",
    OrderStatusNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Order Type",
    ORDER_TYPE_UUID, ":/icons/sort_up_down.png", "", OrderTypeNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(
    PORTFOLIO_ENTRY_RECORD_UUID, ":/icons/chart_line.png", "",
    RecordNode(GetPortfolioEntryRecordType())));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(
    PREVIOUS_UUID, ":/icons/calculator.png", "", PreviousNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(QUERY_UUID,
    ":/icons/chart_line.png", "", QueryNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(QUOTE_RECORD_UUID,
    ":/icons/chart_line.png", "", RecordNode(GetQuoteRecordType())));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(RANGE_UUID,
    ":/icons/calculator.png", "", RangeNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(REFERENCE_UUID,
    ":/icons/bookmark_blue.png", "", ReferenceNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(ROUND_UUID,
    ":/icons/calculator.png", "", RoundNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Security",
    SECURITY_UUID, ":/icons/chart_line.png", "", SecurityNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Security Portfolio",
    SECURITY_PORTFOLIO_UUID, ":/icons/chart_line.png", "",
    SecurityPortfolioNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Side", SIDE_UUID,
    ":/icons/sort_up_down.png", "", SideNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(
    SINGLE_ORDER_TASK_UUID, ":/icons/flash_yellow.png", "",
    SingleOrderTaskNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(SPAWN_TASK_UUID,
    ":/icons/flash_yellow.png", "", SpawnNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(SUBTRACTION_UUID,
    ":/icons/calculator.png", "", SubtractionNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Text", TEXT_UUID,
    ":/icons/window_font.png", "", TextNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(
    TIME_AND_SALE_QUERY_UUID, ":/icons/chart_line.png", "",
    TimeAndSaleQueryNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Time of Day",
    DURATION_UUID, ":/icons/date_time.png", "", TimeNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Time In Force",
    TIME_IN_FORCE_UUID, ":/icons/window_time.png", "", TimeInForceNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(TIMER_UUID,
    ":/icons/hourglass.png", "", TimerNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Time Range",
    TIME_RANGE_PARAMETER_UUID, ":/icons/date_time.png", "",
    TimeRangeParameterNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(UNTIL_TASK_UUID,
    ":/icons/flash_yellow.png", "", UntilNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>("Venue",
    MARKET_UUID, ":/icons/shopping_basket.png", "", VenueNode()));
  entries.push_back(std::make_unique<BuiltInCatalogEntry>(WHEN_TASK_UUID,
    ":/icons/flash_yellow.png", "", WhenNode()));
  return entries;
}

const string& BuiltInCatalogEntry::GetBuiltInSourceValue() {
  static string value = "BuiltInCatalogEntry";
  return value;
}

BuiltInCatalogEntry::BuiltInCatalogEntry(const string& name,
    const uuid& uid, const string& iconPath, const string& description,
    const CanvasNode& node)
    : CatalogEntry(uid),
      m_name(name),
      m_iconPath(iconPath),
      m_icon(QString::fromStdString(iconPath)),
      m_description(description) {
  CanvasNodeBuilder builder(node);
  stringstream ss;
  ss << GetUid();
  builder.SetMetaData(node, CatalogEntry::GetCatalogEntryKey(), ss.str());
  builder.SetMetaData(node, CatalogEntry::GetCatalogEntrySourceKey(),
    BuiltInCatalogEntry::GetBuiltInSourceValue());
  m_node = builder.Make();
}

BuiltInCatalogEntry::BuiltInCatalogEntry(const uuid& uid,
    const string& iconPath, const string& description, const CanvasNode& node)
    : CatalogEntry(uid),
      m_name(node.GetText()),
      m_iconPath(iconPath),
      m_icon(QString::fromStdString(iconPath)),
      m_description(description) {
  CanvasNodeBuilder builder(node);
  stringstream ss;
  ss << GetUid();
  builder.SetMetaData(node, CatalogEntry::GetCatalogEntryKey(), ss.str());
  builder.SetMetaData(node, CatalogEntry::GetCatalogEntrySourceKey(),
    BuiltInCatalogEntry::GetBuiltInSourceValue());
  m_node = builder.Make();
}

string BuiltInCatalogEntry::GetName() const {
  return m_name;
}

unique_ptr<CatalogEntry> BuiltInCatalogEntry::SetName(
    const string& name) const {
  BOOST_THROW_EXCEPTION(NotSupportedException("BuiltInCatalogEntry::SetName"));
}

const string& BuiltInCatalogEntry::GetIconPath() const {
  return m_iconPath;
}

unique_ptr<CatalogEntry> BuiltInCatalogEntry::SetIconPath(
    const string& iconPath) const {
  BOOST_THROW_EXCEPTION(NotSupportedException(
    "BuiltInCatalogEntry::SetIconPath"));
}

QIcon BuiltInCatalogEntry::GetIcon() const {
  return m_icon;
}

string BuiltInCatalogEntry::GetDescription() const {
  return m_description;
}

const CanvasNode& BuiltInCatalogEntry::GetNode() const {
  return *m_node;
}

unique_ptr<CatalogEntry> BuiltInCatalogEntry::SetNode(
    const CanvasNode& node) const {
  BOOST_THROW_EXCEPTION(NotSupportedException("BuiltInCatalogEntry::SetNode"));
}

bool BuiltInCatalogEntry::IsReadOnly() const {
  return true;
}
