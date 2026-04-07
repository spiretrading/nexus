#include "Spire/CanvasView/CutNodeCommand.hpp"
#include <Beam/Collections/View.hpp>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/CanvasView/DeleteNodeCommand.hpp"
#include "Spire/LegacyUI/IgnoreCommandException.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

CutNodeCommand::CutNodeCommand(Ref<CanvasNodeModel> view,
    const CanvasNodeModel::Coordinate& coordinate)
    : m_isInitialRedo(true),
      m_view(view.get()),
      m_coordinate(coordinate),
      m_deleteCommand(Ref(view), coordinate) {}

void CutNodeCommand::undo() {
  m_deleteCommand.undo();
}

void CutNodeCommand::redo() {
  if(m_isInitialRedo) {
    m_isInitialRedo = false;
    auto optionalNode = m_view->GetNode(m_coordinate);
    if(!optionalNode.is_initialized() || optionalNode->IsReadOnly()) {
      BOOST_THROW_EXCEPTION(IgnoreCommandException());
    }
    vector<const CanvasNode*> nodes;
    nodes.push_back(&*optionalNode);
    auto data = EncodeAsMimeData(make_dereference_view(nodes));
    QApplication::clipboard()->setMimeData(data.release());
  }
  m_deleteCommand.redo();
}
