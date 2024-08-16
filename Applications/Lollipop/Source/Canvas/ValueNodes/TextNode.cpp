#include "Spire/Canvas/ValueNodes/TextNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

TextNode::TextNode() {
  SetText("\"" + GetValue() + "\"");
}

TextNode::TextNode(string value)
    : ValueNode(std::move(value)) {
  SetText("\"" + GetValue() + "\"");
}

unique_ptr<TextNode> TextNode::SetValue(string value) const {
  auto clone = CanvasNode::Clone(*this);
  clone->SetInternalValue(std::move(value));
  clone->SetText("\"" + clone->GetValue() + "\"");
  return clone;
}

void TextNode::Apply(CanvasNodeVisitor& visitor) const {
  visitor.Visit(*this);
}

unique_ptr<CanvasNode> TextNode::Clone() const {
  return make_unique<TextNode>(*this);
}

unique_ptr<CanvasNode> TextNode::Reset() const {
  return make_unique<TextNode>();
}
