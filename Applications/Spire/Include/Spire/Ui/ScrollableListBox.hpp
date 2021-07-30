#ifndef SPIRE_SCROLLABLE_LIST_BOX_HPP
#define SPIRE_SCROLLABLE_LIST_BOX_HPP
#include <any>
#include <QLayoutItem>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Represents a ListView contained within a ScrollBox.
   */
  class ScrollableListBox : public QWidget {
    public:

      /**
       * Constructs a ScrollableListBox.
       * @param list_view The ListView contained within the ScrollBox.
       * @param parent The parent widget.
       */
      explicit ScrollableListBox(ListView* list_view, QWidget* parent = nullptr);

      QSize sizeHint() const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      ListView* m_list_view;
      QWidget* m_body;
      ScrollBox* m_scroll_box;
      QLayoutItem* m_scroll_bar_padding;
      int m_padding_size;
      boost::signals2::scoped_connection m_list_view_current_connection;

      void update_ranges();
      void on_current(const boost::optional<std::any>& current);
      bool is_horizontal_layout() const;
  };
}

#endif
