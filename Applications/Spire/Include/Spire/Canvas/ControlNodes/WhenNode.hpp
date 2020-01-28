#ifndef SPIRE_WHEN_NODE_HPP
#define SPIRE_WHEN_NODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  /** Evaluates a CanvasNode when a condition is true. */
  class WhenNode : public FunctionNode {
    public:

      /** Specifies a WhenNode's signatures. */
      struct Signatures {
        template<typename T>
        struct MakeSignature {
          using type = typename boost::mpl::vector<bool, T, T>::type;
        };

        using type = boost::mpl::transform<NativeTypes,
          MakeSignature<boost::mpl::placeholders::_1>>::type;
      };

      /** Constructs a WhenNode. */
      WhenNode();

      void Apply(CanvasNodeVisitor& visitor) const override;

    protected:
      std::unique_ptr<CanvasNode> Clone() const override;

    private:
      friend struct Beam::Serialization::DataShuttle;

      WhenNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void WhenNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam::Serialization {
  template<>
  struct IsDefaultConstructable<Spire::WhenNode> : std::false_type {};
}

#endif
