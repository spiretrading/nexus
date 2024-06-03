#include <QApplication>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/SingleSelectionModel.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TableSelectionModel.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto table = std::make_shared<ArrayTableModel>();
  for(auto i = 0; i != 100; ++i) {
    table->push({i});
  }
  auto current = std::make_shared<LocalValueModel<optional<TableIndex>>>();
  auto selection = std::make_shared<TableSelectionModel>(
    std::make_shared<TableEmptySelectionModel>(),
    std::make_shared<ListSingleSelectionModel>(),
    std::make_shared<ListEmptySelectionModel>());
  auto widths = std::make_shared<ArrayListModel<int>>();
  auto body = new TableBody(
    table, current, selection, widths, TableView::default_item_builder);
  update_style(*body, [] (auto& style) {
    style.get(Any()).
      set(VerticalSpacing(30));//.
//      set(grid_color(QColor(0xFF0000)));
    style.get(Any() > Row() > is_a<TableItem>() > is_a<TextBox>()).
      set(border(1, QColor(0x0000FF)));
  });
  auto scroll_box = new ScrollBox(body);
  scroll_box->resize(300, 100);
  scroll_box->show();
  application.exec();
}
