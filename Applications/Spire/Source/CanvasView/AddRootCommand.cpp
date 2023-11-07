#include "Spire/CanvasView/AddRootCommand.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

using namespace Beam;
using namespace Spire;

AddRootCommand::AddRootCommand(Ref<CanvasNodeModel> view,
  const CanvasNodeModel::Coordinate& coordinate, const CanvasNode& node)
  : m_view(view.Get()),
    m_coordinate(coordinate),
    m_node(CanvasNode::Clone(node)) {}

void AddRootCommand::undo() {
  m_snapshot.Restore(Store(*m_view));
}

void AddRootCommand::redo() {
  m_snapshot.Save(*m_view);
  auto& placedNode = m_view->Add(m_coordinate, *m_node);
  m_view->SetCurrent(placedNode);
}
