#include <QApplication>
#include <QTimer>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto list = std::make_shared<ArrayListModel<QString>>();
  for(auto i = 0; i != 1000; ++i) {
    list->push(QString::number(i));
  }
  auto box = ScrollableListBox(*(new ListView(list)));
  box.resize(box.sizeHint().width(), 100);
  box.show();
  auto timer = QTimer();
  timer.setInterval(10);
  auto i = 0;
  QObject::connect(&timer, &QTimer::timeout, [&] {
    box.get_scroll_box().scroll_to(
      *box.get_list_view().get_list_item(i % list->get_size()));
    ++i;
  });
  timer.start();
  application.exec();
}
