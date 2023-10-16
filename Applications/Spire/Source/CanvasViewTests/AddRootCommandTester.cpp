#include <doctest/doctest.h>
#include <boost/optional/optional_io.hpp>
#include "Spire/Canvas/ControlNodes/AggregateNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeStructuralEquality.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/CanvasView/AddRootCommand.hpp"
#include "Spire/CanvasView/LocalCanvasNodeModel.hpp"

#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/StandardNodes/MultiplicationNode.hpp"
#include "Spire/Canvas/StandardNodes/TimerNode.hpp"

using namespace Beam;
using namespace Spire;

TEST_SUITE("AddRootCommand") {
  TEST_CASE("no_children") {
    auto model = LocalCanvasNodeModel();
    auto node = IntegerNode(123);
    auto command =
      AddRootCommand(Ref(model), CanvasNodeModel::Coordinate(12, 6), node);
    command.redo();
    REQUIRE(model.GetRoots().size() == 1);
    auto modelNode = model.GetRoots().front();
    REQUIRE(IsStructurallyEqual(*modelNode, IntegerNode(123)));
    REQUIRE(model.GetCurrentCoordinate() == CanvasNodeModel::Coordinate(12, 6));
    command.undo();
    REQUIRE(model.GetRoots().empty());
  }

  TEST_CASE("single_child") {
    auto model = LocalCanvasNodeModel();
    auto children = std::vector<std::unique_ptr<CanvasNode>>();
    children.push_back(std::make_unique<IntegerNode>(543));
    auto root = AggregateNode(std::move(children));
    auto command =
      AddRootCommand(Ref(model), CanvasNodeModel::Coordinate(3, 9), root);
    command.redo();
    REQUIRE(model.GetRoots().size() == 1);
    auto modelNode = model.GetRoots().front();
    REQUIRE(IsStructurallyEqual(*modelNode, root));
    REQUIRE(model.GetCurrentCoordinate() == CanvasNodeModel::Coordinate(3, 9));
    command.undo();
    REQUIRE(model.GetRoots().empty());
  }

  TEST_CASE("many_children") {
    auto model = LocalCanvasNodeModel();
    auto children = std::vector<std::unique_ptr<CanvasNode>>();
    for(auto i = 322; i != 548; ++i) {
      children.push_back(std::make_unique<IntegerNode>(i));
    }
    auto root = AggregateNode(std::move(children));
    auto command =
      AddRootCommand(Ref(model), CanvasNodeModel::Coordinate(6, 22), root);
    command.redo();
    REQUIRE(model.GetRoots().size() == 1);
    auto modelNode = model.GetRoots().front();
    REQUIRE(IsStructurallyEqual(*modelNode, root));
    REQUIRE(model.GetCurrentCoordinate() == CanvasNodeModel::Coordinate(6, 22));
    command.undo();
    REQUIRE(model.GetRoots().empty());
  }

  TEST_CASE("m") {
    auto model = LocalCanvasNodeModel();
    auto root =
      std::unique_ptr<CanvasNode>(std::make_unique<SingleOrderTaskNode>());
    auto timer = std::make_unique<TimerNode>();
    auto expression =
      std::unique_ptr<CanvasNode>(std::make_unique<MultiplicationNode>());
    expression = expression->Replace("left", std::make_unique<IntegerNode>(0));
    expression = expression->Replace("right", std::move(timer));
    expression = expression->Convert(MoneyType::GetInstance());
//    root = root->Replace("price", std::move(expression));
  }
}
