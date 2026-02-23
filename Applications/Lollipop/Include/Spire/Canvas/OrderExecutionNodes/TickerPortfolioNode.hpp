#ifndef SPIRE_TICKER_PORTFOLIO_NODE_HPP
#define SPIRE_TICKER_PORTFOLIO_NODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class TickerPortfolioNode
      \brief The CanvasNode representation of a TickerPortfolioExpression.
   */
  class TickerPortfolioNode : public CanvasNode {
    public:

      //! Constructs a TickerPortfolioNode.
      TickerPortfolioNode();

      virtual std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void TickerPortfolioNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
  }
}

#endif
