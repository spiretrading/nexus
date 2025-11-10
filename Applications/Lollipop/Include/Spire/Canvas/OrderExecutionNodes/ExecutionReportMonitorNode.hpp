#ifndef SPIRE_EXECUTIONREPORTMONITORNODE_HPP
#define SPIRE_EXECUTIONREPORTMONITORNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class ExecutionReportMonitorNode
      \brief The CanvasNode representation of an ExecutionReportMonitor.
   */
  class ExecutionReportMonitorNode : public CanvasNode {
    public:

      //! Constructs a ExecutionReportMonitorNode.
      ExecutionReportMonitorNode();

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
  void ExecutionReportMonitorNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
  }
}

#endif
