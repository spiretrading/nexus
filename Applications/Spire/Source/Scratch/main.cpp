#include <QApplication>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"
#include "Spire/Ui/RecycledListViewBuilder.hpp"

using namespace boost;
using namespace Spire;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto list = std::make_shared<ArrayListModel<int>>();
  for(auto i = 0; i != 1000; ++i) {
    list->push(i);
  }
  auto list_view =
    new ListView(list, RecycledListViewBuilder<ListModel<int>>());
  auto scroll_box = new ScrollableListBox(*list_view);
  scroll_box->resize(300, 100);
  scroll_box->show();
  application.exec();
}
