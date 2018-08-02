#ifndef SNAPSHOT_LOADING_WIDGET_HPP
#define SNAPSHOT_LOADING_WIDGET_HPP
#include <QLabel>
#include <QScrollArea>
#include "spire/time_and_sales/time_and_sales.hpp"

namespace Spire {

  //! Displays a widget with an animated loading icon.
  class SnapshotLoadingWidget : public QWidget {
    public:

      //! Constructs a SnapshotLoadingWidget.
      /*
        \param scroll_area The reference scroll area, used to size the
               animated logo widget.
        \param parent The parent widget.
      */
      SnapshotLoadingWidget(QScrollArea* scroll_area,
        QWidget* parent = nullptr);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      QScrollArea* m_scroll_area;
      QLabel* m_logo_widget;

      void update_logo_geometry(int x_pos, int width);
      void on_scroll_bar_value_changed();
  };
}

#endif
