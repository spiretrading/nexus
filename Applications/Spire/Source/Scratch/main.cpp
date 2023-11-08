#include <QApplication>
#include <QTimer>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto model = std::make_shared<ArrayTableModel>();
  auto view = TableViewBuilder(model).
    add_header_item("A").
    make();
  view->show();
  auto timer = QTimer();
  timer.setInterval(100);
  auto counter = 0;
  QObject::connect(&timer, &QTimer::timeout, [&] {
    model->push({counter});
    ++counter;
  });
  timer.start();
  application->exec();
}
