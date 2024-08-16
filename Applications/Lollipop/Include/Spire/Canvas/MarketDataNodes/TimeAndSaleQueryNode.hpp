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
      friend struct Beam::Serialization::DataShuttle;

      TimeAndSaleQueryNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void TimeAndSaleQueryNode::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::TimeAndSaleQueryNode> :
    std::false_type {};
}
}

#endif
