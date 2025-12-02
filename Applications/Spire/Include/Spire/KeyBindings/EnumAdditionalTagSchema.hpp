#ifndef SPIRE_ENUM_ADDITIONAL_TAG_SCHEMA_HPP
#define SPIRE_ENUM_ADDITIONAL_TAG_SCHEMA_HPP
#include <vector>
#include "Spire/KeyBindings/AdditionalTagSchema.hpp"

namespace Spire {

  /** Implements a schema that allows selecting one among a list of values. */
  class EnumAdditionalTagSchema : public AdditionalTagSchema {
    public:

      /**
       * Constructs an EnumAdditionalTagSchema.
       * @param order_field_model The model describing this tag.
       * @param key The tag's key.
       */
      EnumAdditionalTagSchema(
        OrderFieldInfoTip::Model order_field_model, int key);

      bool test(const AdditionalTag& tag) const override;
      AnyInputBox* make_input_box(
        std::shared_ptr<AdditionalTagValueModel> current,
        const SubmitSignal::slot_type& submission) const override;
      std::unique_ptr<CanvasNode> make_canvas_node(
        const boost::optional<Nexus::Tag::Type>& value) const override;
  };
}

#endif
