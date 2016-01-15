#ifndef SPIRE_BBOQUOTENODE_HPP
#define SPIRE_BBOQUOTENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class BboQuoteNode
      \brief Provides a Security's real time BboQuote.
   */
  class BboQuoteNode : public CanvasNode {
    public:

      //! Constructs a BboQuoteNode.
      BboQuoteNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      BboQuoteNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void BboQuoteNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::BboQuoteNode> : std::false_type {};
}
}

#endif
