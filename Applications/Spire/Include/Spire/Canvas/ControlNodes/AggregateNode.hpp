#ifndef SPIRE_AGGREGATENODE_HPP
#define SPIRE_AGGREGATENODE_HPP
#include <vector>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class AggregateNode
      \brief Aggregates a collection of CanvasNodes together.
   */
  class AggregateNode : public CanvasNode {
    public:

      //! Constructs an AggregateNode.
      AggregateNode();

      //! Constructs an AggregateNode.
      /*!
        \param nodes The list of CanvasNodes to aggregate.
      */
      AggregateNode(std::vector<std::unique_ptr<CanvasNode>> nodes);

      virtual std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

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

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::AggregateNode> : std::false_type {};
}
}

#endif
