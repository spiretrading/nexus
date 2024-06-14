#ifndef SPIRE_ADDITIONAL_TAGS_BOX_HPP
#define SPIRE_ADDITIONAL_TAGS_BOX_HPP
#include <vector>
#include <QWidget>
#include "Spire/KeyBindings/AdditionalTag.hpp"
#include "Spire/KeyBindings/AdditionalTagDatabase.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/ClickObserver.hpp"
#include "Spire/Ui/DestinationBox.hpp"
#include "Spire/Ui/RegionBox.hpp"
#include "Spire/Ui/TextBox.hpp"

namespace Spire {

  /** Models a list of AdditionalTags. */
  using AdditionalTagsModel = ValueModel<std::vector<AdditionalTag>>;

  /** Represents an input box for a list of AdditionalTags. */
  class AdditionalTagsBox : public QWidget {
    public:

      /**
       * Signals the submission of the additional tags.
       * @param submission The list of additional tags submitted.
       */
      using SubmitSignal =
        Signal<void (const std::vector<AdditionalTag>& submission)>;

      /**
       * Constructs an AdditionalTagsBox.
       * @param current The list of additional tags to represent.
       * @param additional_tags The definitions of all additional tags.
       * @param destination The destination to constrain the available tags to.
       * @param region The region to constrain the available tags to.
       * @param parent The parent widget.
       */
      AdditionalTagsBox(std::shared_ptr<AdditionalTagsModel> current,
        AdditionalTagDatabase additional_tags,
        std::shared_ptr<DestinationModel> destination,
        std::shared_ptr<RegionModel> region,
        QWidget* parent = nullptr);

      /** Returns the list of additional tags represented. */
      const std::shared_ptr<AdditionalTagsModel>& get_current() const;

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

    private:
      AdditionalTagDatabase m_additional_tags;
      std::shared_ptr<DestinationModel> m_destination;
      std::shared_ptr<RegionModel> m_region;
      std::shared_ptr<AdditionalTagsModel> m_current;
      std::shared_ptr<TextModel> m_tags_text;
      TextBox* m_label;
      bool m_is_read_only;
      ClickObserver m_click_observer;

      void on_click();
  };
}

#endif
