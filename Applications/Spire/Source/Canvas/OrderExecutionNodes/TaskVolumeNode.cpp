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

std::unique_ptr<CanvasNode> Spire::MakeTaskVolumeNode() {
  auto taskReference = std::make_unique<ReferenceNode>(
    "<<<<task", OrderReferenceType::GetInstance());
  auto executionReportMonitor =
    std::unique_ptr<CanvasNode>(std::make_unique<ExecutionReportMonitorNode>());
  executionReportMonitor = executionReportMonitor->Replace(
    executionReportMonitor->GetChildren().front(), std::move(taskReference));
  auto quantityQuery = std::make_unique<QueryNode>();
  quantityQuery = StaticCast<std::unique_ptr<QueryNode>>(quantityQuery->Replace(
    quantityQuery->GetChildren().front(), std::move(executionReportMonitor)));
  quantityQuery = quantityQuery->SetField("last_quantity");
  auto additionCombiner =
    std::unique_ptr<CanvasNode>(std::make_unique<AdditionNode>());
  additionCombiner = additionCombiner->Replace(
    additionCombiner->GetChildren().front(), std::make_unique<IntegerNode>());
  additionCombiner = additionCombiner->Replace(
    additionCombiner->GetChildren().back(), std::make_unique<IntegerNode>());
  auto foldNode = std::unique_ptr<CanvasNode>(std::make_unique<FoldNode>());
  foldNode = foldNode->Replace(
    foldNode->GetChildren().front(), std::move(additionCombiner));
  foldNode =
    foldNode->Replace(foldNode->GetChildren().back(), std::move(quantityQuery));
  auto chainNode = std::unique_ptr<CanvasNode>(std::make_unique<ChainNode>());
  chainNode = chainNode->Replace(
    chainNode->GetChildren().back(), std::make_unique<IntegerNode>(0));
  chainNode =
    chainNode->Replace(chainNode->GetChildren().back(), std::move(foldNode));
  auto sourceParameter =
    std::make_unique<ReferenceNode>("", OrderReferenceType::GetInstance());
  auto volumeChild = CustomNode::Child("volume", IntegerType::GetInstance());
  auto taskChild = CustomNode::Child("task", OrderReferenceType::GetInstance());
  auto customNode = std::unique_ptr<CanvasNode>(
    std::make_unique<CustomNode>(std::string("Volume"),
      std::vector<CustomNode::Child>{volumeChild, taskChild}));
  customNode = customNode->Replace(
    customNode->GetChildren().front(), std::move(chainNode));
  customNode = customNode->Replace(
    customNode->GetChildren().back(), std::move(sourceParameter));
  return customNode;
}
