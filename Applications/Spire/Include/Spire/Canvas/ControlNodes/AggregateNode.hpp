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
      AggregateNode(std::vector<std::unique_ptr<CanvasNode>> nodes);

      std::unique_ptr<CanvasNode>
        Convert(const CanvasType& type) const override;

      std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const override;

      void Apply(CanvasNodeVisitor& visitor) const override;

    protected:
      std::unique_ptr<CanvasNode> Clone() const override;

    private:
      friend struct Beam::Serialization::DataShuttle;

      AggregateNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void AggregateNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

namespace Beam::Serialization {
  template<>
  struct IsDefaultConstructable<Spire::AggregateNode> : std::false_type {};
}

#endif
