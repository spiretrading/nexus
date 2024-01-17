#include <QApplication>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
/*
  auto list = std::make_shared<ArrayListModel<QString>>();
  for(auto i = 0; i != 300; ++i) {
    list->push(QString::number(i));
  }
  auto box = ScrollableListBox(*(new ListView(list)));
  box.resize(box.sizeHint().width(), 100);
  update_style(box.get_list_view(), [&] (auto& style) {
    style.get(Any() > Any()).
      set(border_size(0)).
      set(vertical_padding(0));
  });
  box.show();
*/
  auto parent = QWidget();
  update_style(parent, [] (auto& style) {
    style.get(Any() > Any()).
      set(border_size(0)).
      set(vertical_padding(0));
  });
  auto items = std::vector<ListItem*>();
  for(auto i = 0; i != 300; ++i) {
    auto item = new ListItem(&parent);
    items.push_back(item);
  }
  parent.show();
  for(auto i = 0; i != 100000; ++i) {
    auto w = items[i % 300];
    w->mount(new QWidget());
    w->unmount();
  }
  application.exec();
}
