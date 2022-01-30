#include <QApplication>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/ArrayTableModel.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto model = std::make_shared<ArrayTableModel>();
  for(auto row = 0; row != 50; ++row) {
    auto values = std::vector<std::any>();
    for(auto column = 0; column != 4; ++column) {
      values.push_back(row * 4 + column);
    }
    model->push(values);
  }
  auto header = std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
  auto item = TableHeaderItem::Model();
  item.m_name = "Security";
  header->push(item);
  item = TableHeaderItem::Model();
  item.m_name = "Quantity";
  item.m_order = TableHeaderItem::Order::ASCENDING;
  header->push(item);
  item = TableHeaderItem::Model();
  item.m_name = "Side";
  header->push(item);
  item = TableHeaderItem::Model();
  item.m_name = "Date";
  header->push(item);
  auto start = std::chrono::steady_clock::now();
  auto view = TableViewBuilder(model).
    set_header(header).
    set_standard_filter().
    make();
  view->setFixedHeight(300);
  view->show();
  auto end = std::chrono::steady_clock::now();
  qDebug() <<
    std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
  application->exec();
}
