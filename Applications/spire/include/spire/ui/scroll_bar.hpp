#ifndef SPIRE_SCROLL_BAR_HPP
#define SPIRE_SCROLL_BAR_HPP
#include <QScrollBar>

namespace Spire {

  //! Hides a parent QSCrollBar and displays a scroll bar with a dynamic width.
  class ScrollBar : public QWidget {
    public:

      /*! Constructs a ScrollBar.
        \param scroll_bar The parent and reference for this ScrollBar instance.
      */
      ScrollBar(QScrollBar* scroll_bar);

    protected:
      void paintEvent(QPaintEvent* event) override;

    private:
      QScrollBar* m_parent;
      QWidget* m_track;
      QWidget* m_handle;
      int m_minimum;
      int m_maximum;

      void on_range_changed(int minimum, int maximum);
      void on_value_changed(int value);
  };
}

#endif
