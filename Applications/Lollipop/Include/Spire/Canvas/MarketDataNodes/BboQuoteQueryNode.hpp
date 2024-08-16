#ifndef SPIRE_BBOQUOTENODE_HPP
#define SPIRE_BBOQUOTENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class BboQuoteQueryNode
      \brief Provides a Security's real time BboQuote.
   */
  class BboQuoteQueryNode : public CanvasNode {
    public:

      //! Constructs a BboQuoteQueryNode.
      BboQuoteQueryNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      BboQuoteQueryNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void BboQuoteQueryNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::BboQuoteQueryNode> : std::false_type {};
}
}

#endif
