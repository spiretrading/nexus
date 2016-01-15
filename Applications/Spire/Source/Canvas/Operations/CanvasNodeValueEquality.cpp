#include "Spire/Canvas/Operations/CanvasNodeValueEquality.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Records/QueryNode.hpp"
#include "Spire/Canvas/Types/CanvasType.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

namespace {
  class ValueEqualityVisitor : public CanvasNodeVisitor {
    public:
      bool Test(const CanvasNode& a, const CanvasNode& b);

    private:
      const CanvasNode* m_a;
      bool m_result;

      virtual void Visit(const QueryNode& node);
      virtual void Visit(const CanvasNode& node);
  };

  bool ValueEqualityVisitor::Test(const CanvasNode& a, const CanvasNode& b) {
    m_a = &a;
    b.Apply(*this);
    return m_result;
  }

  void ValueEqualityVisitor::Visit(const CanvasNode& node) {
    if(typeid(*m_a) != typeid(*&node) ||
        m_a->GetType().GetCompatibility(node.GetType()) !=
        CanvasType::Compatibility::EQUAL ||
        m_a->GetChildren().size() != node.GetChildren().size()) {
      m_result = false;
      return;
    }
    for(size_t i = 0; i < m_a->GetChildren().size(); ++i) {
      if(!IsValueEqual(m_a->GetChildren()[i], node.GetChildren()[i])) {
        m_result = false;
        return;
      }
    }
    m_result = true;
  }

  void ValueEqualityVisitor::Visit(const QueryNode& node) {
    const QueryNode* queryNode = dynamic_cast<const QueryNode*>(m_a);
    if(queryNode == nullptr || queryNode->GetField() != node.GetField()) {
      m_result = false;
      return;
    }
    Visit(static_cast<const CanvasNode&>(node));
  }
}

bool Spire::IsValueEqual(const CanvasNode& a, const CanvasNode& b) {
  return ValueEqualityVisitor().Test(a, b);
}
