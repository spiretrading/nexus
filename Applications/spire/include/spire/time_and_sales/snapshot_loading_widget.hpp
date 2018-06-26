#ifndef SNAPSHOT_LOADING_WIDGET_HPP
#define SNAPSHOT_LOADING_WIDGET_HPP
#include <QLabel>
#include <QScrollArea>
#include "spire/time_and_sales/time_and_sales.hpp"

namespace spire {

  //! Displays a widget with an animated loading icon.
  class snapshot_loading_widget : public QWidget {
    public:

      //! Constructs a snapshot_loading_widget.
      /*
        \param scroll_area The reference scroll area, used to size the
          animated logo widget.
        \param parent The parent widget.
      */
      snapshot_loading_widget(QScrollArea* scroll_area,
        QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      QScrollArea* m_scroll_area;
      QLabel* m_logo_widget;

      void update_logo_geometry();
  };
}

#endif
