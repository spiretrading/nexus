#ifndef SPIRE_TAG_COMBO_BOX_HPP
#define SPIRE_TAG_COMBO_BOX_HPP
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a ComboBox using the TagBox as the input box. */
  class TagComboBox : public QWidget {
    public:

      /**
       * Signals the submission of a list of tags.
       * @param submission The list of values to be submitted.
       */
      using SubmitSignal =
        Signal<void (const std::shared_ptr<AnyListModel>& submission)>;

      /**
       * Constructs a TagComboBox using a default local model and a default
       * view builder.
       * @param query_model The model used to query matches.
       * @param parent The parent widget.
       */
      explicit TagComboBox(std::shared_ptr<ComboBox::QueryModel> query_model,
        QWidget* parent = nullptr);

      /**
       * Constructs a TagComboBox using a default local model.
       * @param query_model The model used to query matches.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      TagComboBox(std::shared_ptr<ComboBox::QueryModel> query_model,
        ListViewItemBuilder<> item_builder, QWidget* parent = nullptr);

      /**
       * Constructs a TagComboBox.
       * @param query_model The model used to query matches.
       * @param current The current model which holds a list of tags.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      TagComboBox(std::shared_ptr<ComboBox::QueryModel> query_model,
        std::shared_ptr<AnyListModel> current,
        ListViewItemBuilder<> item_builder, QWidget* parent = nullptr);

      /** Returns the model used to query matches. */
      const std::shared_ptr<ComboBox::QueryModel>& get_query_model() const;

      /** Returns the current list of tags. */
      const std::shared_ptr<AnyListModel>& get_current() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

      /** Returns <code>true</code> iff this TagComboBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the TagComboBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      TagBox* m_tag_box;
      ComboBox* m_combo_box;
      std::shared_ptr<AnyListModel> m_submission;
      FocusObserver m_focus_observer;
      QWidget* m_input_box;
      AnyInputBox* m_any_input_box;
      QWidget* m_drop_down_window;
      bool m_is_modified;
      boost::signals2::scoped_connection m_list_connection;

      void install_text_proxy_event_filter();
      void push_combo_box();
      void submit();
      QWidget* find_drop_down_window();
      void on_combo_box_submit(const std::any& submission);
      void on_focus(FocusObserver::State state);
      void on_operation(const AnyListModel::Operation& operation);
  };
}

#endif
