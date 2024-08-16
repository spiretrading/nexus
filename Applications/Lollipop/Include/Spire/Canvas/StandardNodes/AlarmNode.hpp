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
      friend struct Beam::Serialization::DataShuttle;

      AlarmNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void AlarmNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::AlarmNode> : std::false_type {};
}
}

#endif
