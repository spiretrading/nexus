#ifndef SPIRE_SCROLLABLE_LIST_BOX_HPP
#define SPIRE_SCROLLABLE_LIST_BOX_HPP
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

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

    private:
      ListView* m_list_view;
      ScrollBox* m_scroll_box;
      boost::signals2::scoped_connection m_current_connection;

      void on_current(const boost::optional<int>& current);
  };
}

#endif
