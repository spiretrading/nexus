#ifndef SPIRE_GREATEROREQUALSNODE_HPP
#define SPIRE_GREATEROREQUALSNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  /*! \class GreaterOrEqualsNode
      \brief Tests if one value is greater than or equal to another.
   */
  class GreaterOrEqualsNode : public FunctionNode {
    public:

      //! Constructs a GreaterOrEqualsNode.
      GreaterOrEqualsNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void GreaterOrEqualsNode::shuttle(S& shuttle, unsigned int version) {
    FunctionNode::shuttle(shuttle, version);
  }
}

#endif
