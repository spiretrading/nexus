#include "Spire/Canvas/IONodes/FileReaderNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/IONodes/FilePathNode.hpp"
#include "Spire/Canvas/Operations/CanvasTypeCompatibilityException.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

FileReaderNode::FileReaderNode() {
  SetText("File Reader");
  SetType(UnionType::GetAnyValueType());
  AddChild("path", make_unique<FilePathNode>());
  m_errorPolicy = ParserErrorPolicy::REPORT;
}

FileReaderNode::FileReaderNode(ParserErrorPolicy errorPolicy) {
  SetText("File Reader");
  SetType(UnionType::GetAnyValueType());
  AddChild("path", make_unique<FilePathNode>());
  m_errorPolicy = errorPolicy;
}

FileReaderNode::FileReaderNode(const NativeType& readType)
    : m_readType(readType) {
  SetText("File Reader");
  SetType(*m_readType);
  AddChild("path", make_unique<FilePathNode>());
  m_errorPolicy = ParserErrorPolicy::REPORT;
}

FileReaderNode::FileReaderNode(ParserErrorPolicy errorPolicy,
    const NativeType& readType)
    : m_readType(readType) {
  SetText("File Reader");
  SetType(*m_readType);
  AddChild("path", make_unique<FilePathNode>());
  m_errorPolicy = errorPolicy;
}

ParserErrorPolicy FileReaderNode::GetErrorPolicy() const {
  return m_errorPolicy;
}

unique_ptr<FileReaderNode> FileReaderNode::SetErrorPolicy(
    ParserErrorPolicy errorPolicy) const {
  auto clone = CanvasNode::Clone(*this);
  clone->m_errorPolicy = errorPolicy;
  return clone;
}

boost::optional<const NativeType&> FileReaderNode::GetReadType() const {
  if(m_readType == nullptr) {
    return none;
  }
  return *m_readType;
}

unique_ptr<FileReaderNode> FileReaderNode::SetReadType(
    const NativeType& readType) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetType(readType);
  clone->m_readType = readType;
  return clone;
}

unique_ptr<CanvasNode> FileReaderNode::Convert(const CanvasType& type) const {
  if(m_readType == nullptr) {
    if(auto nativeType = dynamic_cast<const NativeType*>(&type)) {
      auto clone = CanvasNode::Clone(*this);
      clone->SetType(*nativeType);
      return std::move(clone);
    } else {
      return CanvasNode::Convert(type);
    }
  }
  if(IsCompatible(type, *m_readType)) {
    return CanvasNode::Clone(*this);
  }
  BOOST_THROW_EXCEPTION(CanvasTypeCompatibilityException());
}

void FileReaderNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> FileReaderNode::Clone() const {
  return std::make_unique<FileReaderNode>(*this);
}
