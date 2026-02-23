#ifndef SPIRE_INTERACTIONSNODE_HPP
#define SPIRE_INTERACTIONSNODE_HPP
#include "Spire/KeyBindings/InteractionsProperties.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class InteractionsNode
      \brief A CanvasNode used to display InteractionsProperties.
   */
  class InteractionsNode : public CanvasNode {
    public:

      //! Constructs an InteractionsNode.
      InteractionsNode();

      //! Constructs an InteractionsNode.
      /*!
        \param ticker The Ticker whose interactions are represented.
        \param venueDatabase The VenueDatabase containing the
               <i>ticker</i>'s venue.
        \param properties The InteractionsProperties to represent.
      */
      InteractionsNode(Nexus::Ticker ticker,
        const Nexus::VenueDatabase& venueDatabase,
        const InteractionsProperties& properties);

      //! Returns the InteractionsProperties represented.
      const InteractionsProperties& GetProperties() const;

      virtual std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

      using CanvasNode::Replace;
    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;
      InteractionsProperties m_properties;

      void Setup(Nexus::Ticker ticker,
        const Nexus::VenueDatabase& venueDatabase,
        const InteractionsProperties& properties);
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void InteractionsNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
    shuttle.shuttle("properties", m_properties);
  }
}

#endif
