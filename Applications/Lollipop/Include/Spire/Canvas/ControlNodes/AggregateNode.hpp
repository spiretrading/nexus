#ifndef SPIRE_AGGREGATE_NODE_HPP
#define SPIRE_AGGREGATE_NODE_HPP
#include <vector>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /** Aggregates a collection of CanvasNodes together. */
  class AggregateNode : public CanvasNode {
    public:

      /** Constructs an empty AggregateNode. */
      AggregateNode();

      /**
       * Constructs an AggregateNode.
       * @param nodes The list of CanvasNodes to aggregate.
       */
      explicit AggregateNode(std::vector<std::unique_ptr<CanvasNode>> nodes);

      std::unique_ptr<CanvasNode>
        Convert(const CanvasType& type) const override;

      std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const override;

      void Apply(CanvasNodeVisitor& visitor) const override;

    protected:
      std::unique_ptr<CanvasNode> Clone() const override;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void AggregateNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
  }
}

#endif
