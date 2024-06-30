#include <QApplication>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/RecycledTableViewItemBuilder.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto TABLE_VIEW_STYLE() {
    auto style = StyleSheet();
    style.get(Any() > is_a<TableBody>() > Row() > is_a<TableItem>() >
      is_a<Box>()).set(BackgroundColor(0xFF0000));
//    style.get(Any() > (is_a<TableBody>() > (Row() > (is_a<TableItem>() >
//      is_a<Box>())))).set(BackgroundColor(0xFF0000));
    return style;
  }

  struct Builder {
    QWidget* mount(
        const std::shared_ptr<TableModel>& table, int row, int column) {
      auto box = new Box();
      box->setObjectName(QString::fromUtf8("Cell"));
      box->setFixedHeight(scale_height(26));
      return box;
    }

    void reset(QWidget& widget,
      const std::shared_ptr<TableModel>& table, int row, int column) {}

    void unmount(QWidget* widget) {
      delete widget;
    }
  };
}

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto table = std::make_shared<ArrayTableModel>();
  for(auto i = 0; i != 10; ++i) {
    table->push({true});
  }
  auto builder = TableViewBuilder(table);
  builder.add_header_item("R1");
  builder.set_item_builder(RecycledTableViewItemBuilder<Builder>());
  auto view = builder.make();
  set_style(*view, TABLE_VIEW_STYLE());
  view->setFixedHeight(100);
  view->show();
  application.exec();
}
