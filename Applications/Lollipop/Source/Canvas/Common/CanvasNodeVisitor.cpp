#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/CustomNode.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Common/SignatureNode.hpp"
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
#include "Spire/Canvas/OrderExecutionNodes/DefaultCurrencyNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/ExecutionReportMonitorNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderWrapperTaskNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/TickerPortfolioNode.hpp"
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
#include "Spire/Canvas/StandardNodes/FoldOperandNode.hpp"
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
#include "Spire/Canvas/ValueNodes/AssetNode.hpp"
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
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TextNode.hpp"
#include "Spire/Canvas/ValueNodes/TickerNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeRangeNode.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"
#include "Spire/Canvas/ValueNodes/VenueNode.hpp"

using namespace Spire;

void CanvasNodeVisitor::Visit(const AbsNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const AdditionNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const AggregateNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const AlarmNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const AssetNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const BaseValueNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const BboQuoteQueryNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const BlotterTaskMonitorNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const BooleanNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const CanvasNode& node) {}

void CanvasNodeVisitor::Visit(const CeilNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const ChainNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const CountNode& node) {
  Visit(static_cast<const SignatureNode&>(node));
}

void CanvasNodeVisitor::Visit(const CurrencyNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const CurrentDateNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const CurrentDateTimeNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const CurrentTimeNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const CustomNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const DateTimeNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const DecimalNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const DefaultCurrencyNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const DestinationNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const DistinctNode& node) {
  Visit(static_cast<const SignatureNode&>(node));
}

void CanvasNodeVisitor::Visit(const DivisionNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const DurationNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const EqualsNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const ExecutionReportMonitorNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const FilePathNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const FileReaderNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const FilterNode& node) {
  Visit(static_cast<const SignatureNode&>(node));
}

void CanvasNodeVisitor::Visit(const FirstNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const FloorNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const FoldNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const FoldOperandNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const FunctionNode& node) {
  Visit(static_cast<const SignatureNode&>(node));
}

void CanvasNodeVisitor::Visit(const GreaterNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const GreaterOrEqualsNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const IfNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const IntegerNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const InteractionsNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const LastNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const LesserNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const LesserOrEqualsNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const LuaScriptNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const MaxFloorNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const MaxNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const MinNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const MoneyNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const MultiplicationNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const NoneNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const NotNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const OptionalPriceNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const OrderImbalanceQueryNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const OrderStatusNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const OrderTypeNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const OrderWrapperTaskNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const PreviousNode& node) {
  Visit(static_cast<const SignatureNode&>(node));
}

void CanvasNodeVisitor::Visit(const ProxyNode& node) {
  Visit(static_cast<const ReferenceNode&>(node));
}

void CanvasNodeVisitor::Visit(const QueryNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const RangeNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const RecordNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const ReferenceNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const RoundNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const SideNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const SignatureNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const SingleOrderTaskNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const SpawnNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const SubtractionNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const TextNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const TickerPortfolioNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const TickerNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const TimeAndSaleQueryNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const TimeInForceNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const TimeNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const TimeRangeNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const TimeRangeParameterNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const TimerNode& node) {
  Visit(static_cast<const CanvasNode&>(node));
}

void CanvasNodeVisitor::Visit(const UnequalNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const UntilNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}

void CanvasNodeVisitor::Visit(const VenueNode& node) {
  Visit(static_cast<const BaseValueNode&>(node));
}

void CanvasNodeVisitor::Visit(const WhenNode& node) {
  Visit(static_cast<const FunctionNode&>(node));
}
