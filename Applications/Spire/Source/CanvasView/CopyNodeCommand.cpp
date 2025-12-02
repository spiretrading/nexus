#include "Spire/CanvasView/CopyNodeCommand.hpp"
#include <Beam/Collections/View.hpp>
#include <boost/throw_exception.hpp>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/LegacyUI/IgnoreCommandException.hpp"

using namespace Beam;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;

CopyNodeCommand::CopyNodeCommand(const CanvasNode& node)
    : m_copyNode(CanvasNode::Clone(node)) {}

void CopyNodeCommand::undo() {
  assert(false);
}

void CopyNodeCommand::redo() {
  vector<const CanvasNode*> nodes;
  nodes.push_back(m_copyNode.get());
  auto data = EncodeAsMimeData(make_dereference_view(nodes));
  QApplication::clipboard()->setMimeData(data.release());
  BOOST_THROW_EXCEPTION(IgnoreCommandException());
}
