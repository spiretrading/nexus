#ifndef SPIRE_ENUM_ADDITIONAL_TAG_SCHEMA_HPP
#define SPIRE_ENUM_ADDITIONAL_TAG_SCHEMA_HPP
#include <vector>
#include "Spire/KeyBindings/AdditionalTagSchema.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"

namespace Spire {

  /** Implements a schema that allows selecting one among a list of values. */
  class EnumAdditionalTagSchema : public AdditionalTagSchema {
    public:

      /**
       * Constructs an EnumAdditionalTagSchema.
       * @param name The name of the tag.
       * @param key The tag's key.
       * @param The list of cases that can be selected for this tag.
       */
      EnumAdditionalTagSchema(
        std::string name, int key, std::vector<std::string> cases);

      /** Returns the list of cases that can be selected. */
      const std::vector<std::string>& get_cases() const;

      std::unique_ptr<CanvasNode> make_canvas_node(
        const boost::optional<Nexus::Tag::Type>& value) const override;

      AnyInputBox* make_input_box(
        std::shared_ptr<AdditionalTagValueModel> current) const override;

    private:
      std::vector<std::string> m_cases;
  };
}

#endif
