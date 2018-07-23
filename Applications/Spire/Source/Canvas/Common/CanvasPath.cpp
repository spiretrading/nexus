#include "Spire/Canvas/Common/CanvasPath.hpp"
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"

using namespace boost;
using namespace Spire;
using namespace std;

CanvasPath::CanvasPath(const char* fullPath)
    : CanvasPath(string(fullPath)) {}

CanvasPath::CanvasPath(std::string fullPath)
    : m_path(std::move(fullPath)) {}

CanvasPath::CanvasPath(const CanvasNode& node)
    : m_path(GetFullName(node)) {}

const string& CanvasPath::GetPath() const {
  return m_path;
}

const CanvasNode& Spire::GetNode(const CanvasPath& path,
    const CanvasNode& root) {
  if(path.GetPath().empty()) {
    return root;
  }
  auto targetNode = root.FindNode(path.GetPath());
  if(!targetNode.is_initialized()) {
    BOOST_THROW_EXCEPTION(CanvasOperationException("Canvas node not found."));
  }
  return *targetNode;
}

boost::optional<const CanvasNode&> Spire::FindNode(const CanvasPath& path,
    const CanvasNode& root) {
  if(path.GetPath().empty()) {
    return root;
  }
  return root.FindNode(path.GetPath());
}
