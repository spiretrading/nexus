#ifndef SPIRE_TAG_BOX_HPP
#define SPIRE_TAG_BOX_HPP
#include <QWidget>
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/ListModel.hpp"
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

      /**
       * Signals that the tag at the index is being deleted.
       * @param index The index of the deleted tag.
       */
      using DeleteSignal = Signal<void (int index)>;

      /**
       * Constructs a TagBox.
       * @param list The list model which holds a list of tags.
       * @param current The current text's model.
       * @param parent The parent widget.
       */
      TagBox(std::shared_ptr<ListModel> list,
        std::shared_ptr<TextModel> current, QWidget* parent = nullptr);

      /** Returns the list of tags. */
      const std::shared_ptr<ListModel>& get_list() const;

      /** Returns the current text model. */
      const std::shared_ptr<TextModel>& get_current() const;

      /** Returns <code>true</code> iff this TagBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only <code>true</code> iff the TagBox should be
       *        read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the DeleteSignal. */
      boost::signals2::connection connect_delete_signal(
        const DeleteSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void changeEvent(QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable DeleteSignal m_delete_signal;
      struct PartialListModel;
      std::shared_ptr<PartialListModel> m_model;
      bool m_is_read_only;
      QWidget* m_list_view_container;
      ListView* m_list_view;
      TextBox* m_text_box;
      ListItem* m_ellipses_item;
      FocusObserver m_focus_observer;
      std::vector<Tag*> m_tags;
      Styles::TagBoxOverflow m_overflow;
      QFont m_font;
      QMargins m_margins;
      int m_tags_width;
      int m_list_item_gap;
      boost::signals2::scoped_connection m_highlight_connection;
      boost::signals2::scoped_connection m_style_connection;
      boost::signals2::scoped_connection m_list_view_style_connection;
      boost::signals2::scoped_connection m_text_box_style_connection;
      boost::signals2::scoped_connection m_focus_connection;

      QWidget* make_tag(const std::shared_ptr<ListModel>& model, int index);
      void on_focus(FocusObserver::State state);
      void on_operation(const ListModel::Operation& operation);
      void on_submit(const std::any& submission);
      void on_style();
      void on_list_view_style();
      void on_text_box_style();
      void update_tags_read_only();
      void update_tags_width();
      void overflow();
      void reposition_list_view();
      void show_all_tags();
      void add_list_view_to_layout();
      void remove_list_view_from_layout();
      void remove_text_box_width_constraint();
  };
}

#endif
