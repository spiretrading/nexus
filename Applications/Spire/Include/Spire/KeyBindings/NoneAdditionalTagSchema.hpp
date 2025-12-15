#ifndef SPIRE_NONE_ADDITIONAL_TAG_SCHEMA_HPP
#define SPIRE_NONE_ADDITIONAL_TAG_SCHEMA_HPP
#include "Spire/KeyBindings/AdditionalTagSchema.hpp"

namespace Spire {

  /** Implements an AdditionalTagSchema for an invalid, empty, or no tag. */
  class NoneAdditionalTagSchema : public AdditionalTagSchema {
    public:

      /** Returns the single instance of this schema. */
      static const std::shared_ptr<NoneAdditionalTagSchema>& get_instance();

      bool test(const AdditionalTag& tag) const override;
      AnyInputBox* make_input_box(
        std::shared_ptr<AdditionalTagValueModel> current,
        const SubmitSignal::slot_type& submission) const override;
      std::unique_ptr<CanvasNode> make_canvas_node(
        const boost::optional<Nexus::Tag::Type>& value) const override;

    private:
      NoneAdditionalTagSchema();
  };
}

#endif
