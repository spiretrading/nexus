#include "Spire/Canvas/OrderExecutionNodes/TaskVolumeNode.hpp"
#include <Beam/Utilities/Casts.hpp>
#include "Spire/Canvas/Common/CustomNode.hpp"
#include "Spire/Canvas/ControlNodes/ChainNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/ExecutionReportMonitorNode.hpp"
#include "Spire/Canvas/Records/QueryNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/StandardNodes/AdditionNode.hpp"
#include "Spire/Canvas/StandardNodes/FoldNode.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/Types/OrderReferenceType.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

unique_ptr<CanvasNode> Spire::BuildTaskVolumeNode() {
  auto taskReference = make_unique<ReferenceNode>("<<<<task",
    OrderReferenceType::GetInstance());
  unique_ptr<CanvasNode> executionReportMonitor =
    make_unique<ExecutionReportMonitorNode>();
  executionReportMonitor = executionReportMonitor->Replace(
    executionReportMonitor->GetChildren().front(), std::move(taskReference));
  auto quantityQuery = make_unique<QueryNode>();
  quantityQuery = StaticCast<std::unique_ptr<QueryNode>>(quantityQuery->Replace(
    quantityQuery->GetChildren().front(), std::move(executionReportMonitor)));
  quantityQuery = quantityQuery->SetField("last_quantity");
  unique_ptr<CanvasNode> additionCombiner = make_unique<AdditionNode>();
  additionCombiner = additionCombiner->Replace(
    additionCombiner->GetChildren().front(), make_unique<IntegerNode>());
  additionCombiner = additionCombiner->Replace(
    additionCombiner->GetChildren().back(), make_unique<IntegerNode>());
  unique_ptr<CanvasNode> foldNode = make_unique<FoldNode>();
  foldNode = foldNode->Replace(foldNode->GetChildren().front(),
    std::move(additionCombiner));
  foldNode = foldNode->Replace(foldNode->GetChildren().back(),
    std::move(quantityQuery));
  unique_ptr<CanvasNode> chainNode = make_unique<ChainNode>();
  chainNode = chainNode->Replace(chainNode->GetChildren().back(),
    make_unique<IntegerNode>(0));
  chainNode = chainNode->Replace(chainNode->GetChildren().back(),
    std::move(foldNode));
  unique_ptr<CanvasNode> sourceParameter = make_unique<ReferenceNode>("",
    OrderReferenceType::GetInstance());
  CustomNode::Child volumeChild("volume", IntegerType::GetInstance());
  CustomNode::Child taskChild("task", OrderReferenceType::GetInstance());
  unique_ptr<CanvasNode> customNode = make_unique<CustomNode>(
    string{"Volume"}, vector<CustomNode::Child>{volumeChild, taskChild});
  customNode = customNode->Replace(customNode->GetChildren().front(),
    std::move(chainNode));
  customNode = customNode->Replace(customNode->GetChildren().back(),
    std::move(sourceParameter));
  return std::move(customNode);
}
