#include "Spire/Canvas/IONodes/FilePathNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Types/TextType.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

FilePathNode::FilePathNode() {
  SetText("File Path");
  SetType(TextType::GetInstance());
}

FilePathNode::FilePathNode(string path)
    : m_path(std::move(path)) {
  if(m_path.empty()) {
    SetText("File Path");
  } else {
    SetText(m_path);
  }
  SetType(TextType::GetInstance());
}

const string& FilePathNode::GetPath() const {
  return m_path;
}

unique_ptr<FilePathNode> FilePathNode::SetPath(string path) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_path = std::move(path);
  clone->SetText(clone->m_path);
  return clone;
}

void FilePathNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> FilePathNode::Clone() const {
  return make_unique<FilePathNode>(*this);
}

unique_ptr<CanvasNode> FilePathNode::Reset() const {
  return make_unique<FilePathNode>();
}
