#ifndef SPIRE_TAG_BOX_HPP
#define SPIRE_TAG_BOX_HPP
#include <QWidget>
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/GlobalPositionObserver.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/TextBox.hpp"

namespace Spire {
  class InfoTip;
  class ScrollableListBox;
  class ScrollBar;
  class TextAreaBox;

  /** A tag-list widget whose input is AnyInputBox. */
  class AnyTagBox : public QWidget {
    public:

      /** The type of signal emitted when the input is submitted. */
      using SubmitSignal = AnyInputBox::SubmitSignal;

      /**
       * Constructs an AnyTagBox.
       * @param tags The list model which holds a list of tags.
       * @param input_box The input box to use. AnyTagBox takes ownership.
       * @param parent The parent widget.
       */
      AnyTagBox(std::shared_ptr<AnyListModel> tags, AnyInputBox* input_box,
        QWidget* parent = nullptr);

      /** Returns the list of tags. */
      const std::shared_ptr<AnyListModel>& get_tags() const;

      /** Returns the current value model. */
      const std::shared_ptr<AnyValueModel>& get_current() const;

      /** Returns the highlight model. */
      const std::shared_ptr<HighlightModel>& get_highlight() const;

      /** Returns the last submission. */
      const AnyRef& get_submission() const;

      /** Returns the input box. */
      const AnyInputBox& get_input_box() const;

      /** Returns the input box. */
      AnyInputBox& get_input_box();

      /** Sets the placeholder text. */
      void set_placeholder(const QString& placeholder);

      /** Returns <code>true</code> iff this AnyTagBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only <code>true</code> iff the AnyTagBox should be
       *        read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      QSize minimumSizeHint() const override;
      QSize sizeHint() const override;

    protected:

      /**
       * Returns <code>true</code> iff the input has no content under the caret,
       * used to decide whether a backspace removes the trailing tag.
       * The default returns <code>false</code>, and subclasses override to
       * define emptiness for their input type.
       */
      virtual bool is_input_empty() const;

      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;
      void changeEvent(QEvent* event) override;
      bool focusNextPrevChild(bool next) override;
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      struct PartialListModel;
      struct TagItemBuilder;
      std::shared_ptr<PartialListModel> m_model;
      AnyInputBox* m_input_box;
      ListView* m_list_view;
      ScrollableListBox* m_scrollable_list_box;
      ScrollBar* m_horizontal_scroll_bar;
      ScrollBar* m_vertical_scroll_bar;
      TextAreaBox* m_text_area_box;
      InfoTip* m_info_tip;
      QWidget* m_input_focus_proxy;
      FocusObserver m_focus_observer;
      GlobalPositionObserver m_input_box_position_observer;
      boost::optional<Styles::Overflow> m_list_view_overflow;
      QString m_placeholder;
      bool m_is_read_only;
      QMargins m_input_box_border;
      QMargins m_input_box_padding;
      QMargins m_list_view_padding;
      int m_list_item_gap;
      int m_list_overflow_gap;
      int m_min_scroll_height;
      int m_horizontal_scroll_bar_end_range;
      int m_vertical_scroll_bar_end_range;
      bool m_is_transaction;
      boost::signals2::scoped_connection m_style_connection;
      boost::signals2::scoped_connection m_list_view_style_connection;
      boost::signals2::scoped_connection m_text_area_box_style_connection;
      mutable boost::optional<QSize> m_size_hint;

      int get_available_width() const;
      int get_height_for_width(int width) const;
      void set_overflow(Styles::Overflow overflow);
      void install_input_proxy_event_filter();
      void scroll_to_input_box();
      void update_placeholder();
      void update_scroll_bar_end_range(ScrollBar& scroll_bar, int& end_range);
      void update_tags_read_only();
      void update_tip();
      void update_tooltip();
      void update_overflow();
      void update_vertical_scroll_bar_visible();
      void on_focus(FocusObserver::State state);
      void on_operation(const AnyListModel::Operation& operation);
      void on_input_current(const AnyRef& current);
      void on_list_view_current(const boost::optional<int>& current);
      void on_list_view_submit(const std::any& submission);
      void on_style();
      void on_list_view_style();
      void on_text_area_style();
  };

  /** A tag-list widget with a TextBox as its input. */
  class TagBox : public AnyTagBox {
    public:

      /** Signals that the value in the TextBox is being submitted. */
      using SubmitSignal = TextBox::SubmitSignal;

      /**
       * Constructs a TagBox.
       * @param tags The list model which holds a list of tags.
       * @param current The current text's model.
       * @param parent The parent widget.
       */
      TagBox(std::shared_ptr<AnyListModel> tags,
        std::shared_ptr<TextModel> current, QWidget* parent = nullptr);

      /** Returns the current text model. */
      const std::shared_ptr<TextModel>& get_current() const;

      /** Returns the highlight model. */
      const std::shared_ptr<HighlightModel>& get_highlight() const;

      /** Returns the last submission. */
      const QString& get_submission() const;

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool is_input_empty() const override;

    private:
      TextBox* m_text_box;

      TagBox(std::shared_ptr<AnyListModel> tags, TextBox* text_box,
        QWidget* parent);
  };
}

#endif
