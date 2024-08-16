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
      friend struct Beam::Serialization::DataShuttle;

      CurrentDateNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void CurrentDateNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::CurrentDateNode> : std::false_type {};
}
}

#endif
