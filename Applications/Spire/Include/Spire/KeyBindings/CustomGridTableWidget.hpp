#ifndef SPIRE_CUSTOM_GRID_TABLE_WIDGET_HPP
#define SPIRE_CUSTOM_GRID_TABLE_WIDGET_HPP
#include <QTableWidget>

namespace Spire {

  class CustomGridTableWidget : public QTableWidget {
    public:

      CustomGridTableWidget(int row_count, int column_count,
        QWidget* parent = nullptr);

    protected:
      void paintEvent(QPaintEvent* event) override;
  };
}

#endif
