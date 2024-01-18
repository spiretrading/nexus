#include <QApplication>
#include <QTimer>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/ListItem.hpp"

using namespace Spire;
using namespace Spire::Styles;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto parent = QWidget();
  update_style(parent, [] (auto& style) {
    style.get(Any() > Current());
  });
  auto item = ListItem(&parent);
  parent.show();
  auto timer = QTimer();
  timer.setInterval(10);
  auto i = 0;
  timer.connect(&timer, &QTimer::timeout, [&] {
    if(i % 2 == 0) {
      item.mount(*new QWidget());
    } else {
      item.unmount();
    }
    ++i;
  });
  timer.start();
  application.exec();
}
