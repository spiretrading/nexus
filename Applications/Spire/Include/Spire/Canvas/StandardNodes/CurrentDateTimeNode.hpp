#ifndef SPIRE_CURRENTDATETIMENODE_HPP
#define SPIRE_CURRENTDATETIMENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class CurrentDateTimeNode
      \brief Provides the current date and time.
   */
  class CurrentDateTimeNode : public CanvasNode {
    public:

      //! Constructs a CurrentDateTimeNode.
      CurrentDateTimeNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void CurrentDateTimeNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
  }
}

#endif
