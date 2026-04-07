#ifndef SPIRE_UI_SERIALIZATION_HPP
#define SPIRE_UI_SERIALIZATION_HPP
#include <Beam/Serialization/TypeRegistry.hpp>
#include "Spire/Blotter/BlotterWindowSettings.hpp"
#include "Spire/BookView/BookViewWindowSettings.hpp"
#include "Spire/Canvas/Common/CustomNode.hpp"
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
#include "Spire/Canvas/OrderExecutionNodes/DefaultCurrencyNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/ExecutionReportMonitorNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
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
#include "Spire/Canvas/Types/BooleanType.hpp"
#include "Spire/Canvas/Types/CurrencyType.hpp"
#include "Spire/Canvas/Types/DateTimeType.hpp"
#include "Spire/Canvas/Types/DecimalType.hpp"
#include "Spire/Canvas/Types/DestinationType.hpp"
#include "Spire/Canvas/Types/DurationType.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/Canvas/Types/OrderStatusType.hpp"
#include "Spire/Canvas/Types/OrderTypeType.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/SecurityType.hpp"
#include "Spire/Canvas/Types/SequenceType.hpp"
#include "Spire/Canvas/Types/SideType.hpp"
#include "Spire/Canvas/Types/TextType.hpp"
#include "Spire/Canvas/Types/TimeInForceType.hpp"
#include "Spire/Canvas/Types/TimeRangeType.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"
#include "Spire/Canvas/Types/VenueType.hpp"
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
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"
#include "Spire/Canvas/ValueNodes/VenueNode.hpp"
#include "Spire/Charting/ChartIntervalComboBoxWindowSettings.hpp"
#include "Spire/Charting/ChartPlotViewWindowSettings.hpp"
#include "Spire/Charting/ChartWindowSettings.hpp"
#include "Spire/Dashboard/BboPriceDashboardCellBuilder.hpp"
#include "Spire/Dashboard/BboSizeDashboardCellBuilder.hpp"
#include "Spire/Dashboard/ChangeDashboardCellBuilder.hpp"
#include "Spire/Dashboard/CloseDashboardCellBuilder.hpp"
#include "Spire/Dashboard/HighDashboardCellBuilder.hpp"
#include "Spire/Dashboard/DashboardWindowSettings.hpp"
#include "Spire/Dashboard/DashboardWidgetWindowSettings.hpp"
#include "Spire/Dashboard/LastPriceDashboardCellBuilder.hpp"
#include "Spire/Dashboard/ListDashboardRowBuilder.hpp"
#include "Spire/Dashboard/LowDashboardCellBuilder.hpp"
#include "Spire/Dashboard/ValueDashboardCellBuilder.hpp"
#include "Spire/Dashboard/VolumeDashboardCellBuilder.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorVenueSelectionWidgetWindowSettings.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindowSettings.hpp"
#include "Spire/InputWidgets/TimeRangeInputWidgetWindowSettings.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerWindowSettings.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindowSettings.hpp"
#include "Spire/UI/CanvasWindowSettings.hpp"
#include "Spire/UI/CollapsibleWidgetWindowSettings.hpp"
#include "Spire/UI/ToolbarWindowSettings.hpp"

namespace Spire {
  BEAM_REGISTER_TYPES(RegisterCanvasCommonTypes,
    (CustomNode, "Spire.CustomNode"),
    (NoneNode, "Spire.NoneNode"));

  BEAM_REGISTER_TYPES(RegisterCanvasControlNodeTypes,
    (AggregateNode, "Spire.AggregateNode"),
    (ChainNode, "Spire.ChainNode"),
    (SpawnNode, "Spire.SpawnNode"),
    (UntilNode, "Spire.UntilNode"),
    (WhenNode, "Spire.WhenNode"));

  BEAM_REGISTER_TYPES(RegisterCanvasIONodeTypes,
    (FilePathNode, "Spire.FilePathNode"),
    (FileReaderNode, "Spire.FileReaderNode"));

  BEAM_REGISTER_TYPES(RegisterCanvasLuaNodeTypes,
    (LuaScriptNode, "Spire.LuaScriptNode"));

  BEAM_REGISTER_TYPES(RegisterCanvasMarketDataNodeTypes,
    (BboQuoteQueryNode, "Spire.BboQuoteQueryNode"),
    (OrderImbalanceQueryNode, "Spire.OrderImbalanceQueryNode"),
    (TimeAndSaleQueryNode, "Spire.TimeAndSaleQueryNode"));

  BEAM_REGISTER_TYPES(RegisterCanvasOrderExecutionNodeTypes,
    (DefaultCurrencyNode, "Spire.DefaultCurrencyNode"),
    (ExecutionReportMonitorNode, "Spire.ExecutionReportMonitorNode"),
    (MaxFloorNode, "Spire.MaxFloorNode"),
    (OptionalPriceNode, "Spire.OptionalPriceNode"),
    (SecurityPortfolioNode, "Spire.SecurityPortfolioNode"),
    (SingleOrderTaskNode, "Spire.SingleOrderTaskNode"));

  BEAM_REGISTER_TYPES(RegisterCanvasRecordTypes,
    (QueryNode, "Spire.QueryNode"),
    (RecordNode, "Spire.RecordNode"));

  BEAM_REGISTER_TYPES(RegisterCanvasReferenceTypes,
    (ProxyNode, "Spire.ProxyNode"),
    (ReferenceNode, "Spire.ReferenceNode"));

  BEAM_REGISTER_TYPES(RegisterCanvasStandardTypes,
    (AbsNode, "Spire.AbsNode"),
    (AdditionNode, "Spire.AdditionNode"),
    (AlarmNode, "Spire.AlarmNode"),
    (CeilNode, "Spire.CeilNode"),
    (CountNode, "Spire.CountNode"),
    (CurrentDateNode, "Spire.CurrentDateNode"),
    (CurrentDateTimeNode, "Spire.CurrentDateTimeNode"),
    (CurrentTimeNode, "Spire.CurrentTimeNode"),
    (DistinctNode, "Spire.DistinctNode"),
    (DivisionNode, "Spire.DivisionNode"),
    (EqualsNode, "Spire.EqualsNode"),
    (FilterNode, "Spire.FilterNode"),
    (FirstNode, "Spire.FirstNode"),
    (FloorNode, "Spire.FloorNode"),
    (FoldNode, "Spire.FoldNode"),
    (FoldOperandNode, "Spire.FoldOperandNode"),
    (GreaterNode, "Spire.GreaterNode"),
    (GreaterOrEqualsNode, "Spire.GreaterOrEqualsNode"),
    (IfNode, "Spire.IfNode"),
    (LastNode, "Spire.LastNode"),
    (LesserNode, "Spire.LesserNode"),
    (LesserOrEqualsNode, "Spire.LesserOrEqualsNode"),
    (MaxNode, "Spire.MaxNode"),
    (MinNode, "Spire.MinNode"),
    (MultiplicationNode, "Spire.MultiplicationNode"),
    (NotNode, "Spire.NotNode"),
    (NotNode, "Spire.PreviousNode"),
    (RangeNode, "Spire.RangeNode"),
    (RoundNode, "Spire.RoundNode"),
    (SubtractionNode, "Spire.SubtractionNode"),
    (TimeRangeParameterNode, "Spire.TimeRangeParameterNode"),
    (TimerNode, "Spire.TimerNode"),
    (UnequalNode, "Spire.UnequalNode"));

  BEAM_REGISTER_TYPES(RegisterCanvasSystemNodeTypes,
    (BlotterTaskMonitorNode, "Spire.BlotterTaskMonitorNode"),
    (InteractionsNode, "Spire.InteractionsNode"));

  BEAM_REGISTER_TYPES(RegisterCanvasTypeTypes,
    (BooleanType, "Spire.BooleanType"),
    (CurrencyType, "Spire.CurrencyType"),
    (DateTimeType, "Spire.DateTimeType"),
    (DecimalType, "Spire.DecimalType"),
    (DestinationType, "Spire.DestinationType"),
    (DurationType, "Spire.DurationType"),
    (IntegerType, "Spire.IntegerType"),
    (MoneyType, "Spire.MoneyType"),
    (OrderReferenceType, "Spire.OrderReferenceType"),
    (OrderStatusType, "Spire.OrderStatusType"),
    (OrderTypeType, "Spire.OrderTypeType"),
    (RecordType, "Spire.RecordType"),
    (SecurityType, "Spire.SecurityType"),
    (SequenceType, "Spire.SequenceType"),
    (SideType, "Spire.SideType"),
    (TextType, "Spire.TextType"),
    (TimeInForceType, "Spire.TimeInForceType"),
    (TimeRangeType, "Spire.TimeRangeType"),
    (UnionType, "Spire.UnionType"),
    (VenueType, "Spire.VenueType"));

  BEAM_REGISTER_TYPES(RegisterCanvasValueNodeTypes,
    (BooleanNode, "Spire.BooleanNode"),
    (CurrencyNode, "Spire.CurrencyNode"),
    (DateTimeNode, "Spire.DateTimeNode"),
    (DecimalNode, "Spire.DecimalNode"),
    (DestinationNode, "Spire.DestinationNode"),
    (DurationNode, "Spire.DurationNode"),
    (IntegerNode, "Spire.IntegerNode"),
    (MoneyNode, "Spire.MoneyNode"),
    (OrderStatusNode, "Spire.OrderStatusNode"),
    (OrderTypeNode, "Spire.OrderTypeNode"),
    (SecurityNode, "Spire.SecurityNode"),
    (SideNode, "Spire.SideNode"),
    (TextNode, "Spire.TextNode"),
    (TimeInForceNode, "Spire.TimeInForceNode"),
    (TimeNode, "Spire.TimeNode"),
    (TimeRangeNode, "Spire.TimeRangeNode"),
    (VenueNode, "Spire.VenueNode"));

  BEAM_REGISTER_TYPES(RegisterDashboardTypes,
    (BboPriceDashboardCellBuilder, "Spire.BboPriceDashboardCellBuilder"),
    (BboSizeDashboardCellBuilder, "Spire.BboSizeDashboardCellBuilder"),
    (ChangeDashboardCellBuilder, "Spire.ChangeDashboardCellBuilder"),
    (CloseDashboardCellBuilder, "Spire.CloseDashboardCellBuilder"),
    (HighDashboardCellBuilder, "Spire.HighDashboardCellBuilder"),
    (LastPriceDashboardCellBuilder, "Spire.LastPriceDashboardCellBuilder"),
    (ListDashboardRowBuilder, "Spire.ListDashboardRowBuilder"),
    (LowDashboardCellBuilder, "Spire.LowDashboardCellBuilder"),
    (ValueDashboardCellBuilder, "Spire.ValueDashboardCellBuilder"),
    (VolumeDashboardCellBuilder, "Spire.VolumeDashboardCellBuilder"));

  BEAM_REGISTER_TYPES(RegisterWindowSettingsTypes,
    (BlotterWindowSettings, "Spire.BlotterWindowSettings"),
    (BookViewWindowSettings, "Spire.BookViewWindowSettings"),
    (UI::CanvasWindowSettings, "Spire.UI.CanvasWindowSettings"),
    (ChartIntervalComboBoxWindowSettings,
      "Spire.ChartIntervalComboBoxWindowSettings"),
    (ChartPlotViewWindowSettings, "Spire.ChartPlotViewWindowSettings"),
    (ChartWindowSettings, "Spire.ChartWindowSettings"),
    (DashboardWidgetWindowSettings, "Spire.DashboardWidgetWindowSettings"),
    (DashboardWindowSettings, "Spire.DashboardWindowSettings"),
    (UI::CollapsibleWidgetWindowSettings,
      "Spire.UI.CollapsibleWidgetWindowSettings"),
    (OrderImbalanceIndicatorWindowSettings,
      "Spire.OrderImbalanceIndicatorWindowSettings"),
    (OrderImbalanceIndicatorVenueSelectionWidgetWindowSettings,
      "Spire.OrderImbalanceIndicatorVenueSelectionWidgetWindowSettings"),
    (PortfolioViewerWindowSettings, "Spire.PortfolioViewerWindowSettings"),
    (TimeAndSalesWindowSettings, "Spire.TimeAndSalesWindowSettings"),
    (TimeRangeInputWidgetWindowSettings,
      "Spire.TimeRangeInputWidgetWindowSettings"),
    (UI::ToolbarWindowSettings, "Spire.UI.ToolbarWindowSettings"));

  template<Beam::IsSender S>
  void RegisterSpireTypes(Beam::Out<Beam::TypeRegistry<S>> registry) {
    RegisterCanvasCommonTypes(Beam::out(registry));
    RegisterCanvasControlNodeTypes(Beam::out(registry));
    RegisterCanvasIONodeTypes(Beam::out(registry));
    RegisterCanvasLuaNodeTypes(Beam::out(registry));
    RegisterCanvasMarketDataNodeTypes(Beam::out(registry));
    RegisterCanvasOrderExecutionNodeTypes(Beam::out(registry));
    RegisterCanvasRecordTypes(Beam::out(registry));
    RegisterCanvasReferenceTypes(Beam::out(registry));
    RegisterCanvasStandardTypes(Beam::out(registry));
    RegisterCanvasSystemNodeTypes(Beam::out(registry));
    RegisterCanvasTypeTypes(Beam::out(registry));
    RegisterCanvasValueNodeTypes(Beam::out(registry));
    RegisterDashboardTypes(Beam::out(registry));
    RegisterWindowSettingsTypes(Beam::out(registry));
  }
}

#endif
