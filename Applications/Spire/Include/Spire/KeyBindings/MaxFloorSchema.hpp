#ifndef SPIRE_MAX_FLOOR_SCHEMA_HPP
#define SPIRE_MAX_FLOOR_SCHEMA_HPP
#include "Spire/KeyBindings/AdditionalTagSchema.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"

namespace Spire {

  /** Provides a schema for the MaxFloor tag. */
  class MaxFloorSchema : public AdditionalTagSchema {
    public:

      /** Returns the single instance of this schema. */
      static const std::shared_ptr<MaxFloorSchema>& get_instance();

      bool test(const AdditionalTag& tag) const override;

      AnyInputBox* make_input_box(
        std::shared_ptr<AdditionalTagValueModel> current,
        const SubmitSignal::slot_type& submission) const override;

      std::unique_ptr<CanvasNode> make_canvas_node(
        const boost::optional<Nexus::Tag::Type>& value) const override;

    private:
      MaxFloorSchema();
  };
}

#endif
