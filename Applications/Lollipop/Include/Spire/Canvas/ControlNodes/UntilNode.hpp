#ifndef SPIRE_UNTIL_NODE_HPP
#define SPIRE_UNTIL_NODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  /** Evaluates a CanvasNode until a condition is true. */
  class UntilNode : public FunctionNode {
    public:

      /** Specifies an UntilNode's signatures. */
      struct Signatures {
        template<typename T>
        struct MakeSignature {
          using type = typename boost::mpl::vector<bool, T, T>::type;
        };

        using type = boost::mpl::transform<NativeTypes,
          MakeSignature<boost::mpl::placeholders::_1>>::type;
      };

      /** Constructs an UntilNode. */
      UntilNode();

      void Apply(CanvasNodeVisitor& visitor) const override;

    protected:
      std::unique_ptr<CanvasNode> Clone() const override;

    private:
      friend struct Beam::Serialization::DataShuttle;

      UntilNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void UntilNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam::Serialization {
  template<>
  struct IsDefaultConstructable<Spire::UntilNode> : std::false_type {};
}

#endif
