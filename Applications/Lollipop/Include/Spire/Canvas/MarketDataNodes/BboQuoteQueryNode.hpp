#ifndef SPIRE_BBOQUOTENODE_HPP
#define SPIRE_BBOQUOTENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class BboQuoteQueryNode
      \brief Provides a ticker's real time BboQuote.
   */
  class BboQuoteQueryNode : public CanvasNode {
    public:

      //! Constructs a BboQuoteQueryNode.
      BboQuoteQueryNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void BboQuoteQueryNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
  }
}

#endif
