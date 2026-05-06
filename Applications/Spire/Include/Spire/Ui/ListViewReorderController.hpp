#ifndef SPIRE_LIST_VIEW_REORDER_CONTROLLER_HPP
#define SPIRE_LIST_VIEW_REORDER_CONTROLLER_HPP
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/ListView.hpp"

class QMouseEvent;

namespace Spire {

  /** Implements a controller for reordering list items via drag and drop. */
  class ListViewReorderController : public QObject {
    public:

      /**
       * Constructs a ListViewReorderController.
       * @param list_view The ListView to enable reordering on.
       */
      explicit ListViewReorderController(ListView& list_view);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      struct DropItem;
      ListView* m_list_view;
      DropItem* m_drop_item;
      QWidget* m_item_preview;
      ListItem* m_drag_item;
      Qt::Orientation m_direction;
      QPoint m_preview_offset;
      QPoint m_press_position;
      QRect m_last_rect;
      int m_current_index;
      int m_drag_index;
      int m_press_index;
      boost::signals2::scoped_connection m_style_connection;

      int find_cross_index(int current, int direction) const;
      int find_item_index(const QPoint& position) const;
      void start_drag(int index, const QMouseEvent& event);
      void stop_drag();
      void drag_move(const QMouseEvent& event);
      void keyboard_move(int source, int destination);
      void update_drop_item();
      void on_style();
  };
}

#endif
