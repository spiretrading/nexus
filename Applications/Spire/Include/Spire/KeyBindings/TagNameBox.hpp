#ifndef SPIRE_TAG_NAME_BOX_HPP
#define SPIRE_TAG_NAME_BOX_HPP
#include <QPointer>
#include "Spire/KeyBindings/AdditionalTag.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a tag name over a list of available tags. */
  class TagNameBox : public QWidget {
    public:

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal =
        Signal<void (const std::shared_ptr<AdditionalTag>& submission)>;

      /**
       * Constructs a TagNameBox using a default local model.
       * @param query_model The model used to query matches.
       * @param parent The parent widget.
       */
      explicit TagNameBox(std::shared_ptr<ComboBox::QueryModel> query_model,
        QWidget* parent = nullptr);

      /**
       * Constructs a TagNameBox.
       * @param query_model The model used to query matches.
       * @param current The current value's model.
       * @param parent The parent widget.
       */
      TagNameBox(std::shared_ptr<ComboBox::QueryModel> query_model,
        std::shared_ptr<ValueModel<std::shared_ptr<AdditionalTag>>> current,
        QWidget* parent = nullptr);

      /** Returns the model used to query matches. */
      const std::shared_ptr<ComboBox::QueryModel>& get_query_model() const;

      /** Returns the current model. */
      const std::shared_ptr<ValueModel<std::shared_ptr<AdditionalTag>>>&
        get_current() const;

      /** Returns the last submission. */
      const std::shared_ptr<AdditionalTag>& get_submission() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

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
      struct TagNameQueryModel;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<TagNameQueryModel> m_tags;
      std::shared_ptr<ValueModel<std::shared_ptr<AdditionalTag>>> m_current;
      ComboBox* m_combo_box;
      QPointer<OrderFieldInfoTip> m_info_tip;
      std::shared_ptr<AdditionalTag> m_submission;
      boost::signals2::scoped_connection m_connection;

      void on_current(const std::shared_ptr<AdditionalTag>& current);
  };
}

#endif
