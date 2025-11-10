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
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void OrderImbalanceQueryNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
  }
}

#endif
