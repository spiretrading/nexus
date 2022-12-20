#ifndef SPIRE_COMPOSITE_KEY_BINDINGS_MODEL_HPP
#define SPIRE_COMPOSITE_KEY_BINDINGS_MODEL_HPP
#include "Spire/KeyBindings/KeyBindingsModel.hpp"

namespace Spire {

  /**
   * Implements the KeyBindingsModel by composing constituent models together.
   */
  class CompositeKeyBindingsModel : public KeyBindingsModel {
    public:

       /**
       * Constructs a CompositeKeyBindingsModel.
       * @param order_tasks The key bindings of the order task.
       * @param default_order_tasks The default key bindings of the order task.
       * @param region_query_model The model used to query region matches.
       * @param destinations The destination database to use.
       * @param markets The market database to use.
       */
      CompositeKeyBindingsModel(
        std::shared_ptr<ListModel<OrderTask>> order_tasks,
        std::shared_ptr<ListModel<OrderTask>> default_order_tasks,
        std::shared_ptr<ComboBox::QueryModel> region_query_model,
        Nexus::DestinationDatabase destinations,
        Nexus::MarketDatabase markets);

      std::shared_ptr<ListModel<OrderTask>> get_order_tasks() override;

      std::shared_ptr<ListModel<OrderTask>>
        get_default_order_tasks() const override;

      std::shared_ptr<ComboBox::QueryModel> get_region_query_model() override;

      const Nexus::DestinationDatabase& get_destinations() const override;

      const Nexus::MarketDatabase& get_markets() const override;

    private:
      std::shared_ptr<ListModel<OrderTask>> m_order_tasks;
      std::shared_ptr<ListModel<OrderTask>> m_default_order_tasks;
      std::shared_ptr<ComboBox::QueryModel> m_region_query_model;
      Nexus::DestinationDatabase m_destinations;
      Nexus::MarketDatabase m_markets;
  };
}

#endif
