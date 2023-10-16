#include <doctest/doctest.h>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/SignatureNode.hpp"
#include "Spire/Canvas/StandardNodes/TimerNode.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  class TestSignatureNode : public SignatureNode {
    public:
      TestSignatureNode() {
        struct Signatures {
          using type = boost::mpl::list<
            boost::mpl::vector<Quantity, Money, Money>,
            boost::mpl::vector<Money, Quantity, Money>,
            boost::mpl::vector<Quantity, Quantity, Quantity>>;
        };
        m_signatures = MakeSignatures<Signatures>();
        SetText("TestSignature");
        AddChild("p1", MakeDefaultCanvasNode(IntegerType::GetInstance()));
        AddChild("p2", MakeDefaultCanvasNode(MoneyType::GetInstance()));
        SetType(MoneyType::GetInstance());
      }

      void Apply(CanvasNodeVisitor& visitor) const override {
        visitor.Visit(*this);
      }

    protected:
      const std::vector<Signature>& GetSignatures() const override {
        return m_signatures;
      }

      std::unique_ptr<CanvasNode> Clone() const override {
        return std::make_unique<TestSignatureNode>(*this);
      }

    private:
      std::vector<Signature> m_signatures;
  };
}

TEST_SUITE("SignatureNode") {
  TEST_CASE("convert") {
    auto timer = std::make_unique<TimerNode>();
    auto node =
      std::unique_ptr<CanvasNode>(std::make_unique<TestSignatureNode>());
    node = node->Replace("p1", std::make_unique<IntegerNode>(0));
    node = node->Replace("p2", std::move(timer));
    node = node->Convert(MoneyType::GetInstance());
  }
}
