#include <QApplication>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto parent = QWidget();
  auto list = std::make_shared<ArrayListModel<int>>();
  for(auto i = 0; i != 100; ++i) {
    list->push(i);
  }
  auto box = DropDownBox(list);
  enclose(parent, box);
  parent.show();
  application.exec();
}
