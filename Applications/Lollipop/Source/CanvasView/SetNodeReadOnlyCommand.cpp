#include "Spire/CanvasView/SetNodeReadOnlyCommand.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/CanvasView/PlaceNodeCommand.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

SetNodeReadOnlyCommand::SetNodeReadOnlyCommand(Ref<CanvasNodeModel> view,
    const CanvasNodeModel::Coordinate& node, bool readOnly)
    : m_view(view.get()),
      m_node(node),
      m_readOnly(readOnly) {}

void SetNodeReadOnlyCommand::undo() {
  m_snapshot.Restore(out(*m_view));
}

void SetNodeReadOnlyCommand::redo() {
  m_snapshot.Save(*m_view);
  auto node = m_view->GetNode(m_node);
  assert(node);
  auto replacement = node->SetReadOnly(m_readOnly);
  if(IsRoot(*node)) {
    m_view->Remove(*node);
  }
  auto& newNode = PlaceNodeCommand::PlaceNode(out(*m_view), m_node,
    *replacement, true);
  m_view->SetCurrent(newNode);
}
