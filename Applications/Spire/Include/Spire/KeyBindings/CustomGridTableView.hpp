#ifndef SPIRE_CUSTOM_GRID_TABLE_VIEW_HPP
#define SPIRE_CUSTOM_GRID_TABLE_VIEW_HPP
#include <QTableView>

namespace Spire {

  class CustomGridTableView : public QTableView {
    public:

      CustomGridTableView(QWidget* parent = nullptr);

    protected:
      void paintEvent(QPaintEvent* event) override;
  };
}

#endif
