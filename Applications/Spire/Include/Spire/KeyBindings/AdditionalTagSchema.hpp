#ifndef SPIRE_ADDITIONAL_TAG_SCHEMA_HPP
#define SPIRE_ADDITIONAL_TAG_SCHEMA_HPP
#include <memory>
#include <string>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Tag.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/AnyInputBox.hpp"

namespace Spire {

  /** Represents the value of an AdditionalTag. */
  using AdditionalTagValueModel = ValueModel<boost::optional<Nexus::Tag::Type>>;

  /** A local value model over an AdditionalTag's value. */
  using LocalAdditionalTagValueModel =
    LocalValueModel<boost::optional<Nexus::Tag::Type>>;

  /** Base class used to define an additional tag. */
  class AdditionalTagSchema {
    public:
      virtual ~AdditionalTagSchema() = default;

      /** Returns the name of this additional tag. */
      const std::string& get_name() const;

      /** Returns this tag's key. */
      int get_key() const;

      /**
       * Tests if an AdditionalTag satisfies the requirements of this schema.
       * This typically involves ensuring that the tag has the correct type
       * and a valid value, or no value.
       */
      virtual bool test(const AdditionalTag& tag) const = 0;

      /**
       * Makes an input box suitable for representing a value for this type of
       * tag.
       * @param current The tag's current value.
       */
      virtual AnyInputBox* make_input_box(
        std::shared_ptr<AdditionalTagValueModel> current) const = 0;

      /** Makes a CanvasNode representing this tag. */
      virtual std::unique_ptr<CanvasNode> make_canvas_node() const;

      /**
       * Makes a CanvasNode representing this tag and assigned to an initial
       * value.
       * @param value The CanvasNode's initial value.
       */
      virtual std::unique_ptr<CanvasNode> make_canvas_node(
        const boost::optional<Nexus::Tag::Type>& value) const = 0;

    protected:

      /**
       * Constructs an AdditionalTag.
       * @param name The name of the tag.
       * @param key The tag's key.
       */
      AdditionalTagSchema(std::string name, int key);

    private:
      std::string m_name;
      int m_key;

      AdditionalTagSchema(const AdditionalTagSchema&) = delete;
      AdditionalTagSchema& operator =(const AdditionalTagSchema&) = delete;
  };
}

#endif
