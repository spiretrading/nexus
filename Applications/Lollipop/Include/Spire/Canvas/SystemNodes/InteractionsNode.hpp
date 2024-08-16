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
        \param security The Security whose interactions are represented.
        \param marketDatabase The MarketDatabase containing the
               <i>security</i>'s market.
        \param properties The InteractionsProperties to represent.
      */
      InteractionsNode(Nexus::Security security,
        const Nexus::MarketDatabase& marketDatabase,
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
      friend struct Beam::Serialization::DataShuttle;
      InteractionsProperties m_properties;

      void Setup(Nexus::Security security,
        const Nexus::MarketDatabase& marketDatabase,
        const InteractionsProperties& properties);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void InteractionsNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
    shuttle.Shuttle("properties", m_properties);
  }
}

#endif
