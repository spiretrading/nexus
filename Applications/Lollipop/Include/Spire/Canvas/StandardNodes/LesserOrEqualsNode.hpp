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
      friend struct Beam::Serialization::DataShuttle;

      LesserOrEqualsNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void LesserOrEqualsNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::LesserOrEqualsNode> : std::false_type {};
}
}

#endif
