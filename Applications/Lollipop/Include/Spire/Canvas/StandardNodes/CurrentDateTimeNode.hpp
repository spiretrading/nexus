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
      friend struct Beam::Serialization::DataShuttle;

      CurrentDateTimeNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void CurrentDateTimeNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::CurrentDateTimeNode> :
    std::false_type {};
}
}

#endif
