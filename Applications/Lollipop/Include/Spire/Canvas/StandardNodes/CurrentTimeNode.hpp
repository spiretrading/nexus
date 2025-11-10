#ifndef SPIRE_CURRENTTIMENODE_HPP
#define SPIRE_CURRENTTIMENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class CurrentTimeNode
      \brief Provides the current time.
   */
  class CurrentTimeNode : public CanvasNode {
    public:

      //! Constructs a CurrentTimeNode.
      CurrentTimeNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void CurrentTimeNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
  }
}

#endif
