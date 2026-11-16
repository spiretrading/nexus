#include "Spire/Blotter/BlotterTasksTableView.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Spire;

BlotterTasksTableView::BlotterTasksTableView(QWidget* parent)
    : QWidget(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto table = std::make_shared<ArrayTableModel>();
  auto table_view = TableViewBuilder(table).
    add_header_item(tr("Pinned")).
    add_header_item(tr("Name")).
    add_header_item(tr("ID")).
    add_header_item(tr("State")).
    add_header_item(tr("Security")).
    add_header_item(tr("Side")).
    add_header_item(tr("Price"), tr("Px")).
    add_header_item(tr("Quantity"), tr("Qty")).
    add_header_item(tr("Volume"), tr("Vol")).
    set_selection(std::make_shared<TableSelectionModel>(
      std::make_shared<TableEmptySelectionModel>(),
      std::make_shared<ArrayListModel<int>>(),
      std::make_shared<ListEmptySelectionModel>())).
    make();
  table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  enclose(*this, *table_view);
  setFocusProxy(table_view);
}
