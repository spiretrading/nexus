#include <QApplication>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/SingleSelectionModel.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TableSelectionModel.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto p = new QWidget();
  update_style(*p, [] (auto& styles) {
    styles.get(Any() > Any()).set(BackgroundColor(QColor(0xFF00FF)));
  });
  p->show();
  auto s = std::unordered_set<void*>();
  while(true) {
    auto q = new QWidget();
    if(s.contains(q)) {
      qDebug() << q;
    }
    s.insert(q);
    q->setParent(p);
    delete q;
  }
  application.exec();
}
