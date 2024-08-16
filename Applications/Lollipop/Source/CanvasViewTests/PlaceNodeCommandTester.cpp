#include <doctest/doctest.h>
#include <boost/optional/optional_io.hpp>
#include "Spire/Canvas/Operations/CanvasNodeStructuralEquality.hpp"
#include "Spire/Canvas/StandardNodes/AdditionNode.hpp"
#include "Spire/Canvas/StandardNodes/MultiplicationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/CanvasView/AddRootCommand.hpp"
#include "Spire/CanvasView/LocalCanvasNodeModel.hpp"
#include "Spire/CanvasView/PlaceNodeCommand.hpp"

using namespace Beam;
using namespace Spire;

TEST_SUITE("PlaceNodeCommand") {
  TEST_CASE("place_child") {
    auto model = LocalCanvasNodeModel();
    model.Add(CanvasNodeModel::Coordinate(1, 1), AdditionNode());
    auto placeLeftChildCommand = PlaceNodeCommand(
      Ref(model), CanvasNodeModel::Coordinate(1, 2), IntegerNode(321));
    placeLeftChildCommand.redo();
    REQUIRE(model.GetRoots().size() == 1);
    auto placeLeftRoot = model.GetRoots().front();
    auto expectedPlaceLeftRoot = AdditionNode().Replace(
      "left", std::make_unique<IntegerNode>(321));
    REQUIRE(IsStructurallyEqual(*placeLeftRoot, *expectedPlaceLeftRoot));
    auto placeRightChildCommand = PlaceNodeCommand(
      Ref(model), CanvasNodeModel::Coordinate(2, 2), MultiplicationNode());
    placeRightChildCommand.redo();
    auto expectedPlaceRightRoot = expectedPlaceLeftRoot->Replace(
      "right", std::make_unique<MultiplicationNode>());
    auto placeRightRoot = model.GetRoots().front();
    REQUIRE(IsStructurallyEqual(*placeRightRoot, *expectedPlaceRightRoot));
  }
}
