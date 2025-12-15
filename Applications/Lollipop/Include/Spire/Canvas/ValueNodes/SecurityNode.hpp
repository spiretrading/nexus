#ifndef SPIRE_SECURITYNODE_HPP
#define SPIRE_SECURITYNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/SecurityType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class SecurityNode
      \brief Implements the CanvasNode for a Security.
   */
  class SecurityNode : public ValueNode<SecurityType> {
    public:

      //! Constructs a SecurityNode.
      SecurityNode();

      //! Constructs a SecurityNode with an initial value.
      /*!
        \param value The node's initial value.
        \param venueDatabase The database of all available venues.
      */
      SecurityNode(const Nexus::Security& value,
        const Nexus::VenueDatabase& venueDatabase);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \param venueDatabase The database of all available venues.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<SecurityNode> SetValue(const Nexus::Security& value,
        const Nexus::VenueDatabase& venueDatabase) const;

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
  void SecurityNode::shuttle(S& shuttle, unsigned int version) {
    ValueNode<SecurityType>::shuttle(shuttle, version);
  }
}

#endif
