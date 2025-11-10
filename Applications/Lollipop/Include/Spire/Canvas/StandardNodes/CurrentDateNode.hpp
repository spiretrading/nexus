#ifndef SPIRE_CURRENTDATENODE_HPP
#define SPIRE_CURRENTDATENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class CurrentDateNode
      \brief Provides the current date.
   */
  class CurrentDateNode : public CanvasNode {
    public:

      //! Constructs a CurrentDateNode.
      CurrentDateNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void CurrentDateNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
  }
}

#endif
