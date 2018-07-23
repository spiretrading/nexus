#ifndef SPIRE_TASKSTATEMONITORNODE_HPP
#define SPIRE_TASKSTATEMONITORNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class TaskStateMonitorNode
      \brief Monitors the State of a Task.
   */
  class TaskStateMonitorNode : public CanvasNode {
    public:

      //! Constructs a TaskStateMonitorNode.
      TaskStateMonitorNode();

      virtual std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      TaskStateMonitorNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void TaskStateMonitorNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::TaskStateMonitorNode> :
    std::false_type {};
}
}

#endif
