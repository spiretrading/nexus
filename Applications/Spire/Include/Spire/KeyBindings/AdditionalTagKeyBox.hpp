#ifndef SPIRE_ADDITIONAL_TAG_KEY_BOX_HPP
#define SPIRE_ADDITIONAL_TAG_KEY_BOX_HPP
#include <QWidget>
#include "Spire/KeyBindings/AdditionalTagsBox.hpp"
#include "Spire/KeyBindings/AdditionalTagDatabase.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /** Represents the key of an AdditionalTag. */
  using AdditionalTagKeyModel = ValueModel<int>;

  /** A local value model over an AdditionalTag's key. */
  using LocalAdditionalTagKeyModel = LocalValueModel<int>;

  /** Represents the key of an additional tag. */
  class AdditionalTagKeyBox : public QWidget {
    public:

      /**
       * Signals the submission of a key.
       * @param submission The key being submitted.
       */
      using SubmitSignal = Signal<void (int submission)>;

      /**
       * Constructs an AdditionalTagKeyBox.
       * @param current The current tag key to represent.
       * @param additional_tags The definitions of all additional tags.
       * @param destination The destination to constrain the available tags to.
       * @param region The region to constrain the available tags to.
       * @param parent The parent widget.
       */
      AdditionalTagsBox(std::shared_ptr<AdditionalTagKeyModel> current,
        AdditionalTagDatabase additional_tags,
        std::shared_ptr<DestinationModel> destination,
        std::shared_ptr<RegionModel> region,
        QWidget* parent = nullptr);

      /** Returns the current tag key represented. */
      const std::shared_ptr<AdditionalTagKeyModel>& get_current() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

      /** Returns <code>true</code> iff this box is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only <code>true</code> iff the box should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;
  };
}

#endif
