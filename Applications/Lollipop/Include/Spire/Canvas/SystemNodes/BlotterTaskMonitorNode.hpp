#ifndef SPIRE_BLOTTERTASKMONITORNODE_HPP
#define SPIRE_BLOTTERTASKMONITORNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class BlotterTaskMonitorNode
      \brief A CanvasNode used to display Task monitors in a Blotter.
   */
  class BlotterTaskMonitorNode : public CanvasNode {
    public:

      //! Constructs a BlotterTaskMonitorNode.
      BlotterTaskMonitorNode();

      virtual std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

      using CanvasNode::Replace;
    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      BlotterTaskMonitorNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void BlotterTaskMonitorNode::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::BlotterTaskMonitorNode> :
      std::false_type {};
}
}

#endif
