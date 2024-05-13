#ifndef SPIRE_TAG_NAME_BOX_HPP
#define SPIRE_TAG_NAME_BOX_HPP
#include <QPointer>
#include "Spire/KeyBindings/AdditionalTag.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Represents a widget which allows the user to choose a tag name over
   * a list of available tags.
   */
  class TagNameBox : public QWidget {
    public:

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal =
        Signal<void (const std::shared_ptr<AdditionalTag>& submission)>;

      /**
       * Constructs a TagNameBox using a default model.
       * @param tags A list of available tags to display.
       * @param parent The parent widget.
       */
      explicit TagNameBox(
        std::shared_ptr<ListModel<std::shared_ptr<AdditionalTag>>> tags,
        QWidget* parent = nullptr);

      /**
       * Constructs a TagNameBox.
       * @param tags A list of available tags to display.
       * @param current The current value's model.
       * @param parent The parent widget.
       */
      TagNameBox(
        std::shared_ptr<ListModel<std::shared_ptr<AdditionalTag>>> tags,
        std::shared_ptr<ValueModel<std::shared_ptr<AdditionalTag>>> current,
        QWidget* parent = nullptr);

      /** Returns a list of available tags. */
      const std::shared_ptr<ListModel<std::shared_ptr<AdditionalTag>>>&
        get_tags() const;

      /** Returns the current model. */
      const std::shared_ptr<ValueModel<std::shared_ptr<AdditionalTag>>>&
        get_current() const;

      /** Returns <code>true</code> iff the TagNameBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the TagNameBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    private:
      std::shared_ptr<ListModel<std::shared_ptr<AdditionalTag>>> m_tags;
      std::shared_ptr<ValueModel<std::shared_ptr<AdditionalTag>>> m_current;
      DropDownBox* m_drop_down_box;
      QPointer<OrderFieldInfoTip> m_info_tip;
      boost::signals2::scoped_connection m_connection;

      void create_info_tip(const std::shared_ptr<AdditionalTag>& current);
      void on_current(const std::shared_ptr<AdditionalTag>& current);
  };
}

#endif
