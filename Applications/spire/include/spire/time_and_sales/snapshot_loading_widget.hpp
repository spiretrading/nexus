#ifndef SNAPSHOT_LOADING_WIDGET_HPP
#define SNAPSHOT_LOADING_WIDGET_HPP
#include <QLabel>
#include "spire/time_and_sales/time_and_sales.hpp"

namespace spire {

  //! Displays a widget with an animated loading icon.
  class snapshot_loading_widget : public QWidget {
    public:

      //! Constructs a snapshot_loading_widget.
      snapshot_loading_widget(QWidget* parent = nullptr);

    protected:
      void paintEvent(QPaintEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      QLabel* m_logo_widget;
  };
}

#endif
