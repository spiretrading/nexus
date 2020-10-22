#ifndef SPIRE_TAB_WIDGET_HPP
#define SPIRE_TAB_WIDGET_HPP
#include <QTabWidget>

namespace Spire {

  //! Represents a Spire-styled QTabWidget.
  class TabWidget : public QTabWidget {
    public:

      //! Constructs a TabWidget.
      /*!
        \param parent The parent widget.
      */
      TabWidget(QWidget* parent = nullptr);

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

    private:
      bool m_last_focus_was_key;

      void on_tab_bar_clicked(int index);
      void on_tab_changed();
  };
}

#endif
