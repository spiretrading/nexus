#ifndef SPIRE_DESTINATIONNODE_HPP
#define SPIRE_DESTINATIONNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/LinkedNode.hpp"
#include "Spire/Canvas/Types/DestinationType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class DestinationNode
      \brief Implements the CanvasNode for a string value.
   */
  class DestinationNode : public ValueNode<DestinationType>, public LinkedNode {
    public:

      //! Constructs a DestinationNode.
      DestinationNode();

      //! Constructs a DestinationNode with an initial value.
      /*!
        \param value The initial value.
      */
      DestinationNode(std::string value);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<DestinationNode> SetValue(std::string value) const;

      //! Returns the venue being referred to.
      Nexus::Venue GetVenue() const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

      virtual const std::string& GetReferent() const;

      virtual std::unique_ptr<CanvasNode> SetReferent(
        const std::string& referent) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual std::unique_ptr<CanvasNode> Reset() const;

    private:
      friend struct Beam::DataShuttle;
      std::string m_referent;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void DestinationNode::shuttle(S& shuttle, unsigned int version) {
    ValueNode<DestinationType>::shuttle(shuttle, version);
    shuttle.shuttle("referent", m_referent);
  }
}

#endif
