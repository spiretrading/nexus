#ifndef SPIRE_SCROLLABLE_LIST_BOX_HPP
#define SPIRE_SCROLLABLE_LIST_BOX_HPP
#include <QWidget>
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ScrollBox.hpp"

namespace Spire {

  /**
   * Composes a ListView and a ScrollBox together to display a scrollable list
   * of values.
   */
  class ScrollableListBox : public QWidget {
    public:

      /**
       * Constructs a ScrollableListBox.
       * @param list_view The ListView to display within the ScrollBox.
       * @param parent The parent widget.
       */
      explicit ScrollableListBox(
        ListView& list_view, QWidget* parent = nullptr);

      /** Returns the ListView. */
      ListView& get_list_view();

      /** Returns the ScrollBox. */
      ScrollBox& get_scroll_box();

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      struct Styles {
        Qt::Orientation m_direction;
        int m_item_gap;
        int m_overflow_gap;
      };
      ListView* m_list_view;
      QSizePolicy m_size_policy;
      ScrollBox* m_scroll_box;
      bool m_is_showing;
      Styles m_styles;
      boost::signals2::scoped_connection m_list_view_style_connection;
      boost::signals2::scoped_connection m_current_connection;

      void on_list_view_style();
      void on_current(const boost::optional<int>& current);
  };

  /**
   * Selects an index within a ScrollableListBox, sets that index to the current
   * value and scrolls to it.
   * @param list_box The ScrollableListBox to navigate.
   * @param index The index within the <i>list_box</i> to navigate to.
   */
  void navigate_to_index(ScrollableListBox& list_box, int index);

  /**
   * Selects a value in a ScrollableListBox, sets it to the current value, and
   * scrolls to it.
   * @param list_box The ScrollableListBox to navigate.
   * @param value The value within the <i>list_box</i> to navigate to.
   */
  template<typename T>
  void navigate_to_value(ScrollableListBox& list_box, const T& value) {
    auto& list_view = list_box.get_list_view();
    navigate_to_value(list_view, value);
    auto list = std::static_pointer_cast<ListModel<T>>(list_view.get_list());
    auto i = std::find(list->begin(), list->end(), value);
    if(i == list->end()) {
      return;
    }
    auto index = static_cast<int>(std::distance(list->begin(), i));
    list_box.get_scroll_box().scroll_to(*list_view.get_list_item(index));
  }
}

#endif
