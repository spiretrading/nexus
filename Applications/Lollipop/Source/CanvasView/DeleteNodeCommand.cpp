#include "Spire/CanvasView/DeleteNodeCommand.hpp"
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/UI/IgnoreCommandException.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

DeleteNodeCommand::DeleteNodeCommand(Ref<CanvasNodeModel> view,
    const CanvasNodeModel::Coordinate& coordinate)
    : m_view(view.get()),
      m_coordinate(coordinate) {}

void DeleteNodeCommand::undo() {
  m_snapshot.Restore(out(*m_view));
}

void DeleteNodeCommand::redo() {
  auto optionalNode = m_view->GetNode(m_coordinate);
  if(!optionalNode.is_initialized()) {
    BOOST_THROW_EXCEPTION(IgnoreCommandException());
  }
  m_snapshot.Save(*m_view);
  auto& node = *optionalNode;
  if(IsRoot(node)) {
    m_view->Remove(m_coordinate);
  } else if(node.IsReadOnly()) {
    BOOST_THROW_EXCEPTION(IgnoreCommandException());
  } else {
    auto& root = GetRoot(node);
    CanvasNodeBuilder builder(root);
    unique_ptr<CanvasNode> replacement;
    try {
      builder.Reset(node);
      replacement = builder.Make();
    } catch(std::exception&) {
      m_snapshot.Restore(out(*m_view));
      BOOST_THROW_EXCEPTION(IgnoreCommandException());
    }
    auto rootCoordinate = m_view->GetCoordinate(root);
    m_view->Remove(root);
    m_view->Add(rootCoordinate, *replacement);
  }
}
