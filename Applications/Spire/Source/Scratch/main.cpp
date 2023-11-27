#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto parent = QWidget();
  for(auto i = 0; i != 1000; ++i) {
//    auto label = new QLineEdit("hello", &parent);
//    auto label = new QLabel("hello", &parent);
    auto label = make_label("hello", &parent);
  }
  parent.show();
  application.exec();
}
