#include "Spire/CanvasView/ReplaceNodeCommand.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/CanvasView/PlaceNodeCommand.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

const CanvasNode& ReplaceNodeCommand::Replace(Out<CanvasNodeModel> view,
    const CanvasNodeModel::Coordinate& source, const CanvasNode& replacement) {
  auto node = view->GetNode(source);
  assert(node);
  if(IsRoot(*node)) {
    view->Remove(*node);
  }
  return PlaceNodeCommand::PlaceNode(out(*view), source, replacement, false);
}

const CanvasNode& ReplaceNodeCommand::Replace(Out<CanvasNodeModel> view,
    const CanvasNode& source, const CanvasNode& replacement) {
  return Replace(out(view), view->GetCoordinate(source), replacement);
}

ReplaceNodeCommand::ReplaceNodeCommand(Ref<CanvasNodeModel> view,
    const CanvasNodeModel::Coordinate& coordinate, const CanvasNode& node)
    : m_view(view.get()),
      m_coordinate(coordinate),
      m_node(CanvasNode::Clone(node)) {}

void ReplaceNodeCommand::undo() {
  m_snapshot.Restore(out(*m_view));
}

void ReplaceNodeCommand::redo() {
  m_snapshot.Save(*m_view);
  auto& placedNode = Replace(out(*m_view), m_coordinate, *m_node);
  m_view->SetCurrent(placedNode);
}
