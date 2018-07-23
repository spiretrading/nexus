#ifndef SPIRE_ORDERIMBALANCEQUERYNODE_HPP
#define SPIRE_ORDERIMBALANCEQUERYNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class OrderImbalanceQueryNode
      \brief Query's a market's OrderImbalances.
   */
  class OrderImbalanceQueryNode : public CanvasNode {
    public:

      //! Constructs a OrderImbalanceQueryNode.
      OrderImbalanceQueryNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      OrderImbalanceQueryNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void OrderImbalanceQueryNode::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::OrderImbalanceQueryNode> :
    std::false_type {};
}
}

#endif
