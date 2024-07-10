#ifndef SPIRE_ADDITIONAL_TAG_KEY_BOX_HPP
#define SPIRE_ADDITIONAL_TAG_KEY_BOX_HPP
#include <memory>
#include <QWidget>
#include "Spire/KeyBindings/AdditionalTagsBox.hpp"
#include "Spire/KeyBindings/AdditionalTagDatabase.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/Ui.hpp"

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
       * @param available_tags The list of tag keys to choose from.
       * @param additional_tags The definitions of all additional tags.
       * @param destination The destination to constrain the available tags to.
       * @param region The region to constrain the available tags to.
       * @param parent The parent widget.
       */
      AdditionalTagKeyBox(std::shared_ptr<AdditionalTagKeyModel> current,
        std::shared_ptr<ListModel<int>> available_tags,
        AdditionalTagDatabase additional_tags,
        std::shared_ptr<DestinationModel> destination,
        std::shared_ptr<RegionModel> region, QWidget* parent = nullptr);

      /** Returns the current tag key represented. */
      const std::shared_ptr<AdditionalTagKeyModel>& get_current() const;

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
      std::shared_ptr<AdditionalTagKeyModel> m_current;
      boost::signals2::scoped_connection m_connection;
      std::shared_ptr<ListModel<int>> m_available_tags;
      AdditionalTagDatabase m_additional_tags;
      std::shared_ptr<DestinationModel> m_destination;
      std::shared_ptr<RegionModel> m_region;
      DropDownBox* m_drop_down_box;
      std::unique_ptr<OrderFieldInfoTip> m_info_tip;

      void update_info_tip();
      QWidget* make_key_item(
        const std::shared_ptr<ListModel<int>>& available_tags, int index) const;
      QString key_to_text(int key) const;
      void on_current(int key);
  };
}

#endif
