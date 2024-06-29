#include <QApplication>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/RecycledTableViewItemBuilder.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  using DeleteButton = StateSelector<void, struct DeleteButtonSeletorTag>;

  auto TABLE_VIEW_STYLE() {
    auto style = StyleSheet();
    auto body_selector = Any() > is_a<TableBody>();
    auto item_selector = body_selector > Row() > is_a<TableItem>();
    style.get((item_selector > !ReadOnly()) << Current()).
      set(border_color(QColor(Qt::transparent)));
    style.get(body_selector > Row() > Current()).
      set(BackgroundColor(Qt::transparent));
    style.get(body_selector > Row() > HoverItem()).
      set(border_color(QColor(0xA0A0A0)));
    style.get(body_selector > (Row() && Hover())).
      set(BackgroundColor(0xF2F2FF));
    style.get(item_selector > DeleteButton()).
      set(Visibility(Visibility::INVISIBLE));
    style.get(item_selector > DeleteButton() > is_a<Box>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(horizontal_padding(scale_width(2))).
      set(vertical_padding(scale_height(2)));
    style.get(item_selector > DeleteButton() > is_a<Icon>()).
      set(BackgroundColor(QColor(Qt::transparent)));
    style.get(body_selector > (CurrentRow() || (Row() && Hover())) >
        is_a<TableItem>() > DeleteButton()).
      set(Visibility(Visibility::VISIBLE));
    style.get((body_selector > (CurrentRow() || (Row() && Hover()))) >
        DeleteButton() > is_a<Icon>()).
      set(Fill(QColor(0x535353)));
    style.get(body_selector > (Row() && Hover()) > DeleteButton() >
        (is_a<Icon>() && Hover())).
      set(BackgroundColor(QColor(0xDFDFEB))).
      set(Fill(QColor(0xB71C1C)));
    style.get(body_selector > CurrentRow() > DeleteButton() >
        (is_a<Icon>() && Hover())).
      set(BackgroundColor(QColor(0xD0CEEB))).
      set(Fill(QColor(0xB71C1C)));
    style.get(body_selector > CurrentRow()).
      set(BackgroundColor(QColor(0xE2E0FF)));
    style.get(body_selector > CurrentColumn()).
      set(BackgroundColor(Qt::transparent));
    return style;
  }

  struct Builder {
    QWidget* mount(
        const std::shared_ptr<TableModel>& table, int row, int column) {
      auto button = make_delete_icon_button();
      button->setMaximumHeight(scale_height(26));
      match(*button, DeleteButton());
      return button;
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
  builder.set_item_builder(RecycledTableViewItemBuilder<Builder>(Builder()));
  auto view = builder.make();
  set_style(*view, TABLE_VIEW_STYLE());
  view->setFixedHeight(100);
  view->show();
  application.exec();
}
