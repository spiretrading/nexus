#ifndef SPIRE_ADDITIONAL_TAG_HPP
#define SPIRE_ADDITIONAL_TAG_HPP
#include <Beam/Collections/EnumSet.hpp>
#include "Nexus/Definitions/Tag.hpp"
#include "Spire/KeyBindings/OrderFieldInfoTip.hpp"
#include "Spire/Spire/AnyValueModel.hpp"

namespace Spire {

  /** Represents a potentially empty tag value. */
  using AdditionalTagValueModel = ValueModel<boost::optional<Nexus::Tag::Type>>;

  /** Provides the definition of an additional tag. */
  class AdditionalTag {
    public:

      virtual ~AdditionalTag() = default;

      /** Returns the name of this additional tag. */
      const QString& get_name() const;

      /** Returns this tag's key. */
      int get_key() const;

      /** Makes a new value suitable for this tag. */
      virtual const std::shared_ptr<AdditionalTagValueModel>&
        make_value() const = 0;

      /**
       * Makes a new view representing a value suitable for a tag of this type.
       * @param value The value to initialize the view with.
       */
      virtual QWidget* make_view(
        const std::shared_ptr<AdditionalTagValueModel>& value) const = 0;

      /**
       * Makes a new view representing a value suitable for a tag of this type
       * and using a value constructed by calling <i>make_value</i>.
       */
      QWidget* make_view() const;

    protected:

      /**
       * Constructs an AdditionalTag.
       * @param name The name of the tag.
       * @param key The tag's key.
       */
      AdditionalTag(QString name, int key);

    private:
      QString m_name;
      int m_key;

      AdditionalTag(const AdditionalTag&) = delete;
      AdditionalTag& operator =(const AdditionalTag&) = delete;
  };

  /** Converts an AdditionalTag to a Nexus::Tag. */
  Nexus::Tag to_tag(const AdditionalTag& tag);
}

#endif
