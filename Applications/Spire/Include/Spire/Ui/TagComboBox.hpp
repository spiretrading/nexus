#ifndef SPIRE_TAG_COMBO_BOX_HPP
#define SPIRE_TAG_COMBO_BOX_HPP
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/TagBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a ComboBox using the TagBox as the input box. */
  class TagComboBox : public QWidget {
    public:

      /** The type of function used to build a QWidget representing a value. */
      using ViewBuilder = ComboBox::ViewBuilder;

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
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      TagComboBox(std::shared_ptr<ComboBox::QueryModel> query_model,
        ViewBuilder view_builder, QWidget* parent = nullptr);

      /**
       * Constructs a TagComboBox.
       * @param query_model The model used to query matches.
       * @param current The current model which holds a list of tags.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      TagComboBox(std::shared_ptr<ComboBox::QueryModel> query_model,
        std::shared_ptr<AnyListModel> current,
        ViewBuilder view_builder, QWidget* parent = nullptr);

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
      void showEvent(QShowEvent* event) override;
      void moveEvent(QMoveEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      enum class Alignment : std::uint8_t {
        NONE,
        ABOVE,
        BELOW
      };
      mutable SubmitSignal m_submit_signal;
      TagBox* m_tag_box;
      ComboBox* m_combo_box;
      std::shared_ptr<AnyListModel> m_submission;
      FocusObserver m_focus_observer;
      QWidget* m_input_box;
      QWidget* m_parent_window;
      QWidget* m_drop_down_window;
      Styles::TagBoxOverflow m_overflow;
      Alignment m_alignment;
      Alignment m_last_alignment;
      bool m_is_modified;
      bool m_is_internal_move;
      bool m_is_in_layout;
      QPoint m_position;
      QPoint m_bottom_left;
      int m_min_height;
      int m_above_space;
      int m_below_space;
      int m_max_height;
      boost::signals2::scoped_connection m_focus_connection;
      boost::signals2::scoped_connection m_list_connection;
      boost::signals2::scoped_connection m_tag_box_style_connection;

      void on_combo_box_submit(const std::any& submission);
      void on_operation(const AnyListModel::Operation& operation);
      void on_focus(FocusObserver::State state);
      void on_tag_box_style();
      void align();
      void adjust_size();
      void set_position(const QPoint& pos);
      void submit();
      void update_position_and_space();
      void update_position();
      void update_space();
  };
}

#endif
