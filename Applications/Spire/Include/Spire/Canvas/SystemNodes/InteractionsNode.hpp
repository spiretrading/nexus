#ifndef SPIRE_INTERACTIONS_NODE_HPP
#define SPIRE_INTERACTIONS_NODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/KeyBindings/KeyBindingsModel.hpp"

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
        const InteractionsKeyBindingsModel& interactions);

      void Apply(CanvasNodeVisitor& visitor) const override;

    protected:
      std::unique_ptr<CanvasNode> Clone() const override;

    private:
      friend struct Beam::Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  /**
   * Applies the value represented by an <i>InteractionsNode</i> to an
   * <i>InteractionsKeyBindingsModel</i>.
   * @param node The <i>InteractionsNode</i> representing the values to apply.
   * @param interactions The <i>InteractionsKeyBindingsModel</i> to apply the
   *        values from the <i>node</i> to.
   */
  void apply(const InteractionsNode& node,
    InteractionsKeyBindingsModel& interactions);

  /**
   * Applies the value represented by an <i>InteractionsNode</i> to a
   * <i>KeyBindingsModel</i>.
   * @param node The <i>InteractionsNode</i> representing the values to apply.
   * @param keyBindings The <i>KeyBindingsModel</i> to apply the values from the
   *        <i>node</i> to.
   */
  void apply(const InteractionsNode& node, KeyBindingsModel& keyBindings);

  template<typename Shuttler>
  void InteractionsNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

#endif
