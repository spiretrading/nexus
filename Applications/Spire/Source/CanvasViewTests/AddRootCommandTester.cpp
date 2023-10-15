#include <doctest/doctest.h>
#include <boost/optional/optional_io.hpp>
#include "Spire/Canvas/Operations/CanvasNodeStructuralEquality.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/CanvasView/AddRootCommand.hpp"
#include "Spire/CanvasView/LocalCanvasNodeModel.hpp"

using namespace Beam;
using namespace Spire;

TEST_SUITE("AddRootCommand") {
  TEST_CASE("single_node") {
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
}
