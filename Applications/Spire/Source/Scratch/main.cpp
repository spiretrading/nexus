#include <QApplication>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/IntegerBox.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto parent = QWidget();
  for(auto i = 0; i != 1000; ++i) {
    auto label =
      new IntegerBox(std::make_shared<LocalOptionalIntegerModel>(123), &parent);
    label->set_read_only(true);
    apply_label_style(*label);
  }
  parent.show();
  application.exec();
}
