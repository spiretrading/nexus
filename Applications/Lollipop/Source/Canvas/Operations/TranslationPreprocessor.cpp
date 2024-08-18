#include "Spire/Canvas/Operations/TranslationPreprocessor.hpp"
#include <Beam/Utilities/Casts.hpp>
#include <boost/throw_exception.hpp>
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/StandardNodes/FoldNode.hpp"
#include "Spire/Canvas/StandardNodes/FoldOperandNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

namespace {
  class TranslationPreprocessor : private CanvasNodeVisitor {
    public:
      unique_ptr<CanvasNode> Translate(const CanvasNode& node);

      virtual void Visit(const CanvasNode& node);
      virtual void Visit(const FoldNode& node);

    private:
      unique_ptr<CanvasNode> m_result;

      unique_ptr<CanvasNode> DefaultTranslation(const CanvasNode& node);
  };
}

unique_ptr<CanvasNode> Spire::PreprocessTranslation(const CanvasNode& node) {
  TranslationPreprocessor preprocessor;
  return preprocessor.Translate(node);
}

unique_ptr<CanvasNode> TranslationPreprocessor::Translate(
    const CanvasNode& node) {
  node.Apply(*this);
  return std::move(m_result);
}

void TranslationPreprocessor::Visit(const CanvasNode& node) {
  m_result = DefaultTranslation(node);
}

void TranslationPreprocessor::Visit(const FoldNode& node) {
  auto translatedNode = StaticCast<std::unique_ptr<FoldNode>>(
    DefaultTranslation(node));
  if(translatedNode == nullptr) {
    translatedNode = CanvasNode::Clone(node);
  }
  CanvasNodeBuilder builder(*translatedNode);
  unique_ptr<CanvasNode> leftOperand =
    std::make_unique<FoldOperandNode>(FoldOperandNode::Side::LEFT);
  leftOperand = leftOperand->Convert(
    translatedNode->FindLeftOperand()->GetType());
  builder.Replace(GetPath(*translatedNode, *translatedNode->FindLeftOperand()),
    std::move(leftOperand));
  unique_ptr<CanvasNode> rightOperand = std::make_unique<FoldOperandNode>(
    FoldOperandNode::Side::RIGHT);
  rightOperand = rightOperand->Convert(
    translatedNode->FindRightOperand()->GetType());
  builder.Replace(GetPath(*translatedNode, *translatedNode->FindRightOperand()),
    std::move(rightOperand));
  m_result = builder.Make();
}

unique_ptr<CanvasNode> TranslationPreprocessor::DefaultTranslation(
    const CanvasNode& node) {
  CanvasNodeBuilder builder(node);
  auto commitTranslation = false;
  for(auto& child : node.GetChildren()) {
    auto preprocessedChild = PreprocessTranslation(child);
    if(preprocessedChild != nullptr) {
      builder.Replace(GetPath(node, child), std::move(preprocessedChild));
      commitTranslation = true;
    }
  }
  if(commitTranslation) {
    return builder.Make();
  }
  return nullptr;
}
