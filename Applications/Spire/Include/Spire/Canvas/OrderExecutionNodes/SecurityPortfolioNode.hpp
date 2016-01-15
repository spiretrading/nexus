#ifndef SPIRE_SECURITYPORTFOLIONODE_HPP
#define SPIRE_SECURITYPORTFOLIONODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class SecurityPortfolioNode
      \brief The CanvasNode representation of a SecurityPortfolioExpression.
   */
  class SecurityPortfolioNode : public CanvasNode {
    public:

      //! Constructs a SecurityPortfolioNode.
      SecurityPortfolioNode();

      virtual std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      SecurityPortfolioNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void SecurityPortfolioNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::SecurityPortfolioNode> :
    std::false_type {};
}
}

#endif
