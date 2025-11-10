#ifndef SPIRE_ALARMNODE_HPP
#define SPIRE_ALARMNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class AlarmNode
      \brief Evaluates to <code>true</code> after a specified time.
   */
  class AlarmNode : public CanvasNode {
    public:

      //! Constructs a AlarmNode.
      AlarmNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void AlarmNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
  }
}

#endif
