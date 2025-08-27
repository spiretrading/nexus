#ifndef SPIRE_VENUENODE_HPP
#define SPIRE_VENUENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/VenueType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class VenueNode
      \brief Implements the CanvasNode for the VenueType.
   */
  class VenueNode : public ValueNode<VenueType> {
    public:

      //! Constructs a VenueNode.
      VenueNode();

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<VenueNode> SetValue(Nexus::Venue value) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual std::unique_ptr<CanvasNode> Reset() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void VenueNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    ValueNode<VenueType>::Shuttle(shuttle, version);
  }
}

#endif
