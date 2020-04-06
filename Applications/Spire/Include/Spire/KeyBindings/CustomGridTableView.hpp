#ifndef SPIRE_CUSTOM_GRID_TABLE_VIEW_HPP
#define SPIRE_CUSTOM_GRID_TABLE_VIEW_HPP
#include <QTableView>

namespace Spire {

  //! Represents a table view a cutom grid line and item highlight style.
  class CustomGridTableView : public QTableView {
    public:

      //! Constructs a CustomGridTableView.
      /*
        \param parent The parent widget.
      */
      CustomGridTableView(QWidget* parent = nullptr);

    protected:
      void paintEvent(QPaintEvent* event) override;
  };
}

#endif
