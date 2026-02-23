#ifndef SPIRE_DEFAULTCURRENCYNODE_HPP
#define SPIRE_DEFAULTCURRENCYNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class DefaultCurrencyNode
      \brief Evaluates the the default Currency used for a ticker.
   */
  class DefaultCurrencyNode : public CanvasNode {
    public:

      //! Constructs a DefaultCurrencyNode.
      DefaultCurrencyNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void DefaultCurrencyNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
  }
}

#endif
