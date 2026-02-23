#ifndef SPIRE_CANVAS_NODE_VISITOR_HPP
#define SPIRE_CANVAS_NODE_VISITOR_HPP
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /** Implements the visitor pattern for CanvasNodes. */
  class CanvasNodeVisitor {
    public:
      virtual ~CanvasNodeVisitor() = default;

      virtual void Visit(const AbsNode& node);

      virtual void Visit(const AdditionNode& node);

      virtual void Visit(const AggregateNode& node);

      virtual void Visit(const AlarmNode& node);

      virtual void Visit(const BaseValueNode& node);

      virtual void Visit(const BboQuoteQueryNode& node);

      virtual void Visit(const BlotterTaskMonitorNode& node);

      virtual void Visit(const BooleanNode& node);

      virtual void Visit(const CanvasNode& node);

      virtual void Visit(const CeilNode& node);

      virtual void Visit(const ChainNode& node);

      virtual void Visit(const CountNode& node);

      virtual void Visit(const CurrencyNode& node);

      virtual void Visit(const CurrentDateNode& node);

      virtual void Visit(const CurrentDateTimeNode& node);

      virtual void Visit(const CurrentTimeNode& node);

      virtual void Visit(const CustomNode& node);

      virtual void Visit(const DateTimeNode& node);

      virtual void Visit(const DecimalNode& node);

      virtual void Visit(const DefaultCurrencyNode& node);

      virtual void Visit(const DestinationNode& node);

      virtual void Visit(const DistinctNode& node);

      virtual void Visit(const DivisionNode& node);

      virtual void Visit(const DurationNode& node);

      virtual void Visit(const EqualsNode& node);

      virtual void Visit(const ExecutionReportMonitorNode& node);

      virtual void Visit(const FilePathNode& node);

      virtual void Visit(const FileReaderNode& node);

      virtual void Visit(const FilterNode& node);

      virtual void Visit(const FirstNode& node);

      virtual void Visit(const FloorNode& node);

      virtual void Visit(const FoldNode& node);

      virtual void Visit(const FoldOperandNode& node);

      virtual void Visit(const FunctionNode& node);

      virtual void Visit(const GreaterNode& node);

      virtual void Visit(const GreaterOrEqualsNode& node);

      virtual void Visit(const IfNode& node);

      virtual void Visit(const IntegerNode& node);

      virtual void Visit(const InteractionsNode& node);

      virtual void Visit(const LastNode& node);

      virtual void Visit(const LesserNode& node);

      virtual void Visit(const LesserOrEqualsNode& node);

      virtual void Visit(const LuaScriptNode& node);

      virtual void Visit(const MaxFloorNode& node);

      virtual void Visit(const MaxNode& node);

      virtual void Visit(const MinNode& node);

      virtual void Visit(const MoneyNode& node);

      virtual void Visit(const MultiplicationNode& node);

      virtual void Visit(const NoneNode& node);

      virtual void Visit(const NotNode& node);

      virtual void Visit(const OptionalPriceNode& node);

      virtual void Visit(const OrderImbalanceQueryNode& node);

      virtual void Visit(const OrderStatusNode& node);

      virtual void Visit(const OrderTypeNode& node);

      virtual void Visit(const OrderWrapperTaskNode& node);

      virtual void Visit(const PreviousNode& node);

      virtual void Visit(const ProxyNode& node);

      virtual void Visit(const QueryNode& node);

      virtual void Visit(const RangeNode& node);

      virtual void Visit(const RecordNode& node);

      virtual void Visit(const ReferenceNode& node);

      virtual void Visit(const RoundNode& node);

      virtual void Visit(const SideNode& node);

      virtual void Visit(const SignatureNode& node);

      virtual void Visit(const SingleOrderTaskNode& node);

      virtual void Visit(const SpawnNode& node);

      virtual void Visit(const SubtractionNode& node);

      virtual void Visit(const TextNode& node);

      virtual void Visit(const TickerPortfolioNode& node);

      virtual void Visit(const TickerNode& node);

      virtual void Visit(const TimeAndSaleQueryNode& node);

      virtual void Visit(const TimeInForceNode& node);

      virtual void Visit(const TimeNode& node);

      virtual void Visit(const TimeRangeNode& node);

      virtual void Visit(const TimeRangeParameterNode& node);

      virtual void Visit(const TimerNode& node);

      virtual void Visit(const UnequalNode& node);

      virtual void Visit(const UntilNode& node);

      virtual void Visit(const VenueNode& node);

      virtual void Visit(const WhenNode& node);
  };
}

#endif
