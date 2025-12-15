#include "Spire/CanvasView/PlaceNodeCommand.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Operations/CanvasNodeRefresh.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

const CanvasNode& PlaceNodeCommand::PlaceNode(Out<CanvasNodeModel> view,
    const CanvasNodeModel::Coordinate coordinate, const CanvasNode& node,
    bool overrideReadOnly) {
  auto placedNode = TryPlaceNode(out(view), coordinate, node, overrideReadOnly);
  if(!placedNode.is_initialized()) {
    return view->Add(coordinate, *Refresh(CanvasNode::Clone(node)));
  }
  return *placedNode;
}

boost::optional<const CanvasNode&> PlaceNodeCommand::TryPlaceNode(
    Out<CanvasNodeModel> view, const CanvasNodeModel::Coordinate coordinate,
    const CanvasNode& node, bool overrideReadOnly) {
  auto previousNode = view->GetNode(coordinate);
  if(!previousNode.is_initialized()) {
    return view->Add(coordinate, *Refresh(CanvasNode::Clone(node)));
  }
  if(IsRoot(*previousNode) || !overrideReadOnly && previousNode->IsReadOnly()) {
    return none;
  }
  auto& root = GetRoot(*previousNode);
  CanvasNodeBuilder builder(root);
  unique_ptr<CanvasNode> replacement;
  try {
    auto identity = CanvasNodeModel::FindIdentity(*previousNode);
    builder.Replace(*previousNode, CanvasNode::Clone(node));
    if(identity.is_initialized()) {
      builder.SetMetaData(*previousNode, CanvasNodeModel::GetIdentityKey(),
        CanvasNodeModel::ToMetaData(*identity));
    }
    replacement = builder.Make();
  } catch(const std::exception&) {
    return none;
  }
  auto rootCoordinate = view->GetCoordinate(root);
  auto path = GetFullName(*previousNode);
  view->Remove(root);
  auto& replacementRoot = view->Add(rootCoordinate, *Refresh(
    std::move(replacement)));
  return *replacementRoot.FindNode(path);
}

PlaceNodeCommand::PlaceNodeCommand(Ref<CanvasNodeModel> view,
    const CanvasNodeModel::Coordinate& coordinate, const CanvasNode& node)
    : m_view(view.get()),
      m_coordinate(coordinate),
      m_node(CanvasNode::Clone(node)) {}

void PlaceNodeCommand::undo() {
  m_snapshot.Restore(out(*m_view));
}

void PlaceNodeCommand::redo() {
  m_snapshot.Save(*m_view);
  auto& node = PlaceNode(out(*m_view), m_coordinate, *m_node, false);
  m_view->SetCurrent(node);
}
