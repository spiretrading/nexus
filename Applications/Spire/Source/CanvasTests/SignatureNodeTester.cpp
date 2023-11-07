#include <doctest/doctest.h>
#include <Beam/Collections/DereferenceIterator.hpp>
#include <boost/optional/optional_io.hpp>
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Common/SignatureNode.hpp"
#include "Spire/Canvas/StandardNodes/TimerNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeStructuralEquality.hpp"
#include "Spire/Canvas/Operations/CanvasTypeCompatibilityException.hpp"
#include "Spire/Canvas/Operations/DefaultCanvasNodeFromCanvasTypeVisitor.hpp"
#include "Spire/Canvas/StandardNodes/AdditionNode.hpp"
#include "Spire/Canvas/StandardNodes/MultiplicationNode.hpp"
#include "Spire/Canvas/Types/DecimalType.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"

using namespace Beam;
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
        auto type = UnionType::Create(
          MakeDereferenceView(std::vector<std::shared_ptr<NativeType>>{
            MoneyType::GetInstance(), IntegerType::GetInstance()}));
        AddChild("p1", MakeDefaultCanvasNode(*type));
        AddChild("p2", MakeDefaultCanvasNode(*type));
        SetType(*type);
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
    auto node =
      std::unique_ptr<CanvasNode>(std::make_unique<TestSignatureNode>());
    node = node->Convert(IntegerType::GetInstance());
    REQUIRE(node->GetType().GetCompatibility(IntegerType::GetInstance()) ==
      CanvasType::Compatibility::EQUAL);
    auto childP1 = node->FindChild("p1");
    REQUIRE(childP1.has_value());
    REQUIRE(IsStructurallyEqual(*childP1, IntegerNode(0)));
    auto childP2 = node->FindChild("p2");
    REQUIRE(childP2.has_value());
    REQUIRE(IsStructurallyEqual(*childP2, IntegerNode(0)));
  }

  TEST_CASE("replace_then_convert") {
    auto node =
      std::unique_ptr<CanvasNode>(std::make_unique<TestSignatureNode>());
    node = node->Replace("p1", std::make_unique<IntegerNode>(0));
    auto expectedType = UnionType::Create(
      MakeDereferenceView(std::vector<std::shared_ptr<NativeType>>{
        IntegerType::GetInstance(), MoneyType::GetInstance()}));
    REQUIRE(node->GetType().GetCompatibility(*expectedType) ==
      CanvasType::Compatibility::EQUAL);
    node = node->Replace("p2", std::make_unique<TimerNode>());
    REQUIRE(node->GetType().GetCompatibility(IntegerType::GetInstance()) ==
      CanvasType::Compatibility::EQUAL);
    REQUIRE_THROWS_AS(node->Convert(
      MoneyType::GetInstance()), CanvasTypeCompatibilityException);
  }

  TEST_CASE("polymorphic_replace") {
    auto node =
      std::unique_ptr<CanvasNode>(std::make_unique<TestSignatureNode>());
    node = node->Replace("p1", std::make_unique<MoneyNode>(3 * Money::CENT));
    REQUIRE(node->GetType().GetCompatibility(MoneyType::GetInstance()) ==
      CanvasType::Compatibility::EQUAL);
    auto p2 = NoneNode().Convert(*UnionType::Create(
      MakeDereferenceView(std::vector<std::shared_ptr<NativeType>>{
        IntegerType::GetInstance(), MoneyType::GetInstance()})));
    node = node->Replace("p2", std::move(p2));
    REQUIRE(node->GetType().GetCompatibility(MoneyType::GetInstance()) ==
      CanvasType::Compatibility::EQUAL);
    auto childP2 = node->FindChild("p2");
    REQUIRE(childP2.has_value());
    REQUIRE(childP2->GetType().GetCompatibility(IntegerType::GetInstance()) ==
      CanvasType::Compatibility::EQUAL);
    REQUIRE(node->GetChildren().size() == 2);
    REQUIRE(
      IsStructurallyEqual(node->GetChildren()[0], MoneyNode(3 * Money::CENT)));
    REQUIRE(IsStructurallyEqual(node->GetChildren()[1], IntegerNode(0)));
  }

  TEST_CASE("add") {
    auto node =
      std::unique_ptr<CanvasNode>(std::make_unique<AdditionNode>());
    node = node->Replace("left", std::make_unique<IntegerNode>(711));
    node = node->Replace("right", std::make_unique<MultiplicationNode>());
    auto expectedType = UnionType::Create(
      MakeDereferenceView(std::vector<std::shared_ptr<NativeType>>{
        DecimalType::GetInstance(), IntegerType::GetInstance()}));
    REQUIRE(node->GetType().GetCompatibility(*expectedType) ==
        CanvasType::Compatibility::EQUAL);
    REQUIRE(node->GetChildren().size() == 2);
    REQUIRE(
      IsStructurallyEqual(node->GetChildren()[0], IntegerNode(711)));
    REQUIRE(node->GetChildren()[1].GetChildren().size() == 2);
  }
}
