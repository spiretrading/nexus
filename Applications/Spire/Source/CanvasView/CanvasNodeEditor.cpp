#include "Spire/CanvasView/CanvasNodeEditor.hpp"
#include <boost/variant/get.hpp>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/CanvasView/CommitCanvasEditor.hpp"
#include "Spire/CanvasView/OpenCanvasEditor.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;

CanvasNodeEditor::EditVariant CanvasNodeEditor::GetEditor(
    Ref<const CanvasNode> node, Ref<CanvasNodeModel> view,
    Ref<UserProfile> userProfile, QEvent* event) {
  m_node = node.Get();
  if(m_node->IsReadOnly()) {
    return static_cast<QUndoCommand*>(nullptr);
  }
  m_view = view.Get();
  m_userProfile = userProfile.Get();
  m_editVariant = OpenCanvasEditor(*m_node, *m_view, *m_userProfile, event);
  return m_editVariant;
}

QUndoCommand* CanvasNodeEditor::Commit() {
  auto editor = get<QWidget*>(m_editVariant);
  if(editor == nullptr) {
    return nullptr;
  }
  return CommitCanvasEditor(*m_node, *editor, *m_view, *m_userProfile);
}
