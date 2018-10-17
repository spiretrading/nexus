#include "Spire/CanvasView/SetNodeVisibleCommand.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/CanvasView/PlaceNodeCommand.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

SetNodeVisibleCommand::SetNodeVisibleCommand(Ref<CanvasNodeModel> view,
    const CanvasNodeModel::Coordinate& parent, const string& node,
    bool isVisible)
    : m_view(view.Get()),
      m_parent(parent),
      m_node(node),
      m_isVisible(isVisible) {}

void SetNodeVisibleCommand::undo() {
  m_snapshot.Restore(Store(*m_view));
}

void SetNodeVisibleCommand::redo() {
  m_snapshot.Save(*m_view);
  auto parent = m_view->GetNode(m_parent);
  assert(parent);
  auto child = parent->FindChild(m_node);
  assert(child);
  auto replacementChild = child->SetVisible(m_isVisible);
  auto replacement = parent->Replace(*child, std::move(replacementChild));
  if(IsRoot(*parent)) {
    m_view->Remove(*parent);
  }
  auto& newNode = PlaceNodeCommand::PlaceNode(Store(*m_view), m_parent,
    *replacement, true);
  m_view->SetCurrent(newNode);
}
