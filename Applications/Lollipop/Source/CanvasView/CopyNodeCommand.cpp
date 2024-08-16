#include "Spire/CanvasView/CopyNodeCommand.hpp"
#include <Beam/Collections/DereferenceIterator.hpp>
#include <boost/throw_exception.hpp>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/UI/IgnoreCommandException.hpp"

using namespace Beam;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

CopyNodeCommand::CopyNodeCommand(const CanvasNode& node)
    : m_copyNode(CanvasNode::Clone(node)) {}

void CopyNodeCommand::undo() {
  assert(false);
}

void CopyNodeCommand::redo() {
  vector<const CanvasNode*> nodes;
  nodes.push_back(m_copyNode.get());
  auto data = EncodeAsMimeData(MakeDereferenceView(nodes));
  QApplication::clipboard()->setMimeData(data.release());
  BOOST_THROW_EXCEPTION(IgnoreCommandException());
}
