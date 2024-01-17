#include <QApplication>
#include <QTimer>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto list = std::make_shared<ArrayListModel<QString>>();
  for(auto i = 0; i != 20; ++i) {
    list->push(QString::number(i));
  }
  auto parent = QWidget();
  auto box = new ListView(list, &parent);
  update_style(*box, [&] (auto& style) {
    style.get(Any() > Any()).
      set(border_size(0)).
      set(vertical_padding(0));
  });
  box->move(0, 0);
  box->resize(500, 2000);
  parent.resize(500, 1000);
  parent.show();
  auto timer = QTimer();
  timer.setInterval(100);
  auto i = 0;
  timer.connect(&timer, &QTimer::timeout,
    [&] {
      if(i == 1000) {
        update_style(*box, [&] (auto& style) {
          style.get(Any()).set(EdgeNavigation::CONTAIN);
        });
      }
      if(i > 2000) {
//        box->move(0, -(i % 2000));
      }
      i += 200;
    });
  timer.start();
/*
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
  auto timer = QTimer();
  timer.setInterval(100);
  auto i = 0;
  timer.connect(&timer, &QTimer::timeout,
    [&] {
      if(i % 2 == 0) {
        auto w = items[i % 300];
        w->mount(*make_label(QString::number(i)));
      } else {
        auto w = items[(i - 1) % 300];
        w->unmount();
      }
      ++i;
    });
  timer.start();
*/
  application.exec();
}
