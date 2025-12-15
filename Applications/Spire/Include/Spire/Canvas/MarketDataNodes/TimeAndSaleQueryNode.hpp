#ifndef SPIRE_TIMEANDSALEQUERYNODE_HPP
#define SPIRE_TIMEANDSALEQUERYNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class TimeAndSaleQueryNode
      \brief Query's a Security's TimeAndSales.
   */
  class TimeAndSaleQueryNode : public CanvasNode {
    public:

      //! Constructs a TimeAndSaleQueryNode.
      TimeAndSaleQueryNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void TimeAndSaleQueryNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
  }
}

#endif
