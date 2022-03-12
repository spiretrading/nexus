#ifndef SPIRE_TAG_BOX_HPP
#define SPIRE_TAG_BOX_HPP
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Specifies how to layout the tags on overflow. */
  enum class TagBoxOverflow {

    /** Tags will be truncated with ellipsis. */
    ELIDE,

    /** Tags will wrap onto multiple lines if they are too long. */
    WRAP
  };
}

  /** Displays a list of tags within a box. */
  class TagBox : public QWidget {
    public:

       /** Signals that the current value in the TextBox is being submitted. */
      using SubmitSignal = TextBox::SubmitSignal;

      /**
       * Constructs a TagBox.
       * @param list The list model which holds a list of tags.
       * @param current The current text's model.
       * @param parent The parent widget.
       */
      TagBox(std::shared_ptr<AnyListModel> list,
        std::shared_ptr<TextModel> current, QWidget* parent = nullptr);

      /** Returns the list of tags. */
      const std::shared_ptr<AnyListModel>& get_list() const;

      /** Returns the current text model. */
      const std::shared_ptr<TextModel>& get_current() const;

      /** Returns the highlight model. */
      const std::shared_ptr<HighlightModel>& get_highlight() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

      /** Returns <code>true</code> iff this TagBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only <code>true</code> iff the TagBox should be
       *        read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void changeEvent(QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      struct PartialListModel;
      std::shared_ptr<PartialListModel> m_model;
      TextBox* m_text_box;
      ListView* m_list_view;
      ScrollBox* m_scroll_box;
      ScrollBar* m_vertical_scroll_bar;
      ListItem* m_ellipses_item;
      Tooltip* m_tooltip;
      FocusObserver m_focus_observer;
      std::vector<Tag*> m_tags;
      Styles::TagBoxOverflow m_overflow;
      QFont m_font;
      QString m_placeholder;
      QString m_tip;
      bool m_is_read_only;
      int m_tags_width;
      int m_list_item_gap;
      int m_input_box_horizontal_padding;
      int m_list_view_horizontal_padding;
      int m_scroll_bar_end_range;
      boost::signals2::scoped_connection m_style_connection;
      boost::signals2::scoped_connection m_list_view_style_connection;
      boost::signals2::scoped_connection m_text_box_style_connection;
      boost::signals2::scoped_connection m_focus_connection;

      QWidget* make_tag(const std::shared_ptr<AnyListModel>& model, int index);
      void on_focus(FocusObserver::State state);
      void on_operation(const AnyListModel::Operation& operation);
      void on_text_box_current(const QString& current);
      void on_list_view_submit(const std::any& submission);
      void on_style();
      void on_list_view_style();
      void on_text_box_style();
      void update();
      void update_placeholder();
      void update_tags_read_only();
      void update_tags_width();
      void update_tip();
      void update_tooltip();
      void overflow();
      void show_all_tags();
      void remove_text_box_size_constraint();
  };
}

#endif
