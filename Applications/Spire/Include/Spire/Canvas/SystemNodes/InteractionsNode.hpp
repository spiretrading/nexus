#ifndef SPIRE_INTERACTIONS_NODE_HPP
#define SPIRE_INTERACTIONS_NODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"

namespace Spire {

  /** A CanvasNode used to display InteractionsProperties. */
  class InteractionsNode : public CanvasNode {
    public:

      /** Constructs an InteractionsNode using default interactions. */
      InteractionsNode();

      /**
       * Constructs an InteractionsNode.
       * @param security The Security whose interactions are represented.
       * @param marketDatabase The MarketDatabase containing the
       *        <i>security</i>'s market.
       * @param interactions The interactions to represent.
       */
      InteractionsNode(Nexus::Security security,
        const Nexus::MarketDatabase& marketDatabase,
        const std::shared_ptr<InteractionsKeyBindingsModel>& interactions);

      /** Returns the interactions. */
      const std::shared_ptr<InteractionsKeyBindingsModel>&
        GetProperties() const;

      std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const override;

      void Apply(CanvasNodeVisitor& visitor) const override;

      using CanvasNode::Replace;
    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      std::shared_ptr<InteractionsKeyBindingsModel> m_interactions;

      void Setup(Nexus::Security security,
        const Nexus::MarketDatabase& marketDatabase,
        const std::shared_ptr<InteractionsKeyBindingsModel>& interactions);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void InteractionsNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
/** TODO    shuttle.Shuttle("properties", m_properties); */
  }
}

#endif
