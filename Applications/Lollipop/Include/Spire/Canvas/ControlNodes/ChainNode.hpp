#ifndef SPIRE_CHAIN_NODE_HPP
#define SPIRE_CHAIN_NODE_HPP
#include <vector>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /** Evaluates a series of CanvasNodes in order. */
  class ChainNode : public CanvasNode {
    public:

      /** Constructs an empty ChainNode. */
      ChainNode();

      /**
       * Constructs a ChainNode.
       * @param nodes The list of CanvasNodes to chain.
       */
      explicit ChainNode(std::vector<std::unique_ptr<CanvasNode>> nodes);

      std::unique_ptr<CanvasNode>
        Convert(const CanvasType& type) const override;

      std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const override;

      void Apply(CanvasNodeVisitor& visitor) const override;

    protected:
      std::unique_ptr<CanvasNode> Clone() const override;

    private:
      friend struct Beam::Serialization::DataShuttle;

      ChainNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void ChainNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

namespace Beam::Serialization {
  template<>
  struct IsDefaultConstructable<Spire::ChainNode> : std::false_type {};
}

#endif
