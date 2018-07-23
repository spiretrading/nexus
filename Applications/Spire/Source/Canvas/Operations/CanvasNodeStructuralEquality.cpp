#include "Spire/Canvas/Operations/CanvasNodeStructuralEquality.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/Types/CanvasType.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

namespace {
  class StructuralEqualityVisitor : public CanvasNodeVisitor {
    public:
      bool Test(const CanvasNode& a, const CanvasNode& b);

    private:
      const CanvasNode* m_a;
      bool m_result;

      virtual void Visit(const CanvasNode& node);
  };

  bool StructuralEqualityVisitor::Test(const CanvasNode& a,
      const CanvasNode& b) {
    m_a = &a;
    b.Apply(*this);
    return m_result;
  }

  void StructuralEqualityVisitor::Visit(const CanvasNode& node) {
    if(typeid(*m_a) != typeid(*&node) ||
        m_a->GetType().GetCompatibility(node.GetType()) !=
        CanvasType::Compatibility::EQUAL ||
        m_a->GetChildren().size() != node.GetChildren().size()) {
      m_result = false;
      return;
    }
    for(size_t i = 0; i < m_a->GetChildren().size(); ++i) {
      if(!IsStructurallyEqual(m_a->GetChildren()[i], node.GetChildren()[i])) {
        m_result = false;
        return;
      }
    }
    m_result = true;
  }
}

bool Spire::IsStructurallyEqual(const CanvasNode& a, const CanvasNode& b) {
  return StructuralEqualityVisitor().Test(a, b);
}
