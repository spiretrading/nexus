#ifndef SPIRE_SCROLLABLE_LIST_BOX_HPP
#define SPIRE_SCROLLABLE_LIST_BOX_HPP
#include <QLayoutItem>
#include <QWidget>
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Represents a ScrollBox which contains a ListView.
   */
  class ScrollableListBox : public ScrollBox {
    public:

      /**
       * Constructs a ScrollableListBox.
       * @param list_view The ListView contained within the ScrollBox.
       * @param parent The parent widget.
       */
      explicit ScrollableListBox(ListView* list_view, QWidget* parent = nullptr);

      QSize sizeHint() const override;

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void update_ranges() override;

    private:
      ListView* m_list_view;
      QWidget* m_body;
      QLayoutItem* m_scroll_bar_padding;
      int m_padding_size;

      void on_current(const boost::optional<std::any>& current);
      QWidget* make_body();
      bool is_horizontal_layout() const;
      int get_bar_width();
      int get_bar_height();
  };
}

#endif
