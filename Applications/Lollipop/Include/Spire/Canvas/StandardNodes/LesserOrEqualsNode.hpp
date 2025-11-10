#ifndef SPIRE_LESSEROREQUALSNODE_HPP
#define SPIRE_LESSEROREQUALSNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  /*! \class LesserOrEqualsNode
      \brief Tests if one value is less than or equal to another.
   */
  class LesserOrEqualsNode : public FunctionNode {
    public:

      //! Constructs a LesserOrEqualsNode.
      LesserOrEqualsNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void LesserOrEqualsNode::shuttle(S& shuttle, unsigned int version) {
    FunctionNode::shuttle(shuttle, version);
  }
}

#endif
