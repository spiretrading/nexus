#include <QApplication>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"

using namespace Spire;

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
  box.setFixedHeight(100);
  box.show();
  application.exec();
}
