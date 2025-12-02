#ifndef SPIRE_TIMEINFORCENODE_HPP
#define SPIRE_TIMEINFORCENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/TimeInForceType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class TimeInForceNode
      \brief Implements the CanvasNode for a TimeInForce value.
   */
  class TimeInForceNode : public ValueNode<TimeInForceType> {
    public:

      //! Constructs a TimeInForceNode.
      TimeInForceNode();

      //! Constructs a TimeInForceNode.
      /*!
        \param value The initial value.
      */
      TimeInForceNode(const Nexus::TimeInForce& value);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<TimeInForceNode> SetValue(
        const Nexus::TimeInForce& value) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual std::unique_ptr<CanvasNode> Reset() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void TimeInForceNode::shuttle(S& shuttle, unsigned int version) {
    ValueNode<TimeInForceType>::shuttle(shuttle, version);
  }
}

#endif
