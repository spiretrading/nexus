#ifndef SPIRE_DEFAULTCURRENCYNODE_HPP
#define SPIRE_DEFAULTCURRENCYNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class DefaultCurrencyNode
      \brief Evaluates the the default Currency used for a Security.
   */
  class DefaultCurrencyNode : public CanvasNode {
    public:

      //! Constructs a DefaultCurrencyNode.
      DefaultCurrencyNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      DefaultCurrencyNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void DefaultCurrencyNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::DefaultCurrencyNode> :
    std::false_type {};
}
}

#endif
