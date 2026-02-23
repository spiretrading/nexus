#ifndef SPIRE_TICKER_NODE_HPP
#define SPIRE_TICKER_NODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/TickerType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class TickerNode
      \brief Implements the CanvasNode for a Ticker.
   */
  class TickerNode : public ValueNode<TickerType> {
    public:

      //! Constructs a TickerNode.
      TickerNode();

      //! Constructs a TickerNode with an initial value.
      /*!
        \param value The node's initial value.
        \param venueDatabase The database of all available venues.
      */
      TickerNode(const Nexus::Ticker& value,
        const Nexus::VenueDatabase& venueDatabase);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \param venueDatabase The database of all available venues.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<TickerNode> SetValue(const Nexus::Ticker& value,
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
  void TickerNode::shuttle(S& shuttle, unsigned int version) {
    ValueNode<TickerType>::shuttle(shuttle, version);
  }
}

#endif
