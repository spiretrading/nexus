#ifndef SPIRE_TABLE_HEADER_HPP
#define SPIRE_TABLE_HEADER_HPP
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/TableHeaderCell.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
  class TableHeader : public QWidget {
    public:
      explicit TableHeader(
        std::shared_ptr<ListModel<TableHeaderCell::Model>> cells,
        QWidget* parent = nullptr);

      const std::shared_ptr<ListModel<TableHeaderCell::Model>>& get_cells()
        const;

    private:
      std::shared_ptr<ListModel<TableHeaderCell::Model>> m_cells;
  };
}

#endif
