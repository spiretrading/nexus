#ifndef SPIRE_BASIC_ADDITIONAL_TAG_SCHEMA_HPP
#define SPIRE_BASIC_ADDITIONAL_TAG_SCHEMA_HPP
#include "Spire/KeyBindings/AdditionalTagSchema.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"

namespace Spire {

  /**
   * Implements an AdditionalTagSchema using the default CanvasNodes associated
   * with a specified CanvasType.
   */
  class BasicAdditionalTagSchema : public AdditionalTagSchema {
    public:

      /**
       * Constructs a BasicAdditionalTagSchema.
       * @param order_field_model The model describing this tag.
       * @param key The tag's key.
       * @param type The type of the tag.
       */
      BasicAdditionalTagSchema(OrderFieldInfoTip::Model order_field_model,
        int key, const CanvasType& type);

      /**
       * Constructs a BasicAdditionalTagSchema.
       * @param order_field_model The model describing this tag.
       * @param key The tag's key.
       * @param default_value The tag's default value.
       */
      BasicAdditionalTagSchema(OrderFieldInfoTip::Model order_field_model,
        int key, Nexus::Tag::Type default_value);

      bool test(const AdditionalTag& tag) const override;

      std::unique_ptr<CanvasNode> make_canvas_node(
        const boost::optional<Nexus::Tag::Type>& value) const override;

      AnyInputBox* make_input_box(
        std::shared_ptr<AdditionalTagValueModel> current) const override;

    private:
      std::shared_ptr<CanvasType> m_type;
      boost::optional<Nexus::Tag::Type> m_default_value;
  };
}

#endif
