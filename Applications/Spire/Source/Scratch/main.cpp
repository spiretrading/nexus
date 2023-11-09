#include <QApplication>
#include <QTimer>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/TextBox.hpp"

#include <QTableWidget>
#include <QLineEdit>
#include <QHeaderView>
#include <cstdlib>
#include <ctime>

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
/*
   QTableWidget tableWidget(100, 10); // 10 rows and 10 columns

    // Set column headers
    QStringList headers;
    for (int col = 0; col < 10; ++col) {
        headers << QString("A%1").arg(col);
    }
    tableWidget.setHorizontalHeaderLabels(headers);

    // Seed the random number generator
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    for (int row = 0; row < tableWidget.rowCount(); ++row) {
        for (int col = 0; col < tableWidget.columnCount(); ++col) {
            // Generate a random integer between 0 and 99
            int randomValue = std::rand() % 100;
//            QLineEdit *lineEdit = new QLineEdit(QString::number(randomValue));
//            lineEdit->setReadOnly(true); // Set the QLineEdit to read-only
//            tableWidget.setCellWidget(row, col, lineEdit);


            tableWidget.setCellWidget(row, col, make_label(QString::number(randomValue)));
        }
    }

    // Make the table look nicer
    tableWidget.horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget.verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    tableWidget.show();
*/
  auto model = std::make_shared<ArrayTableModel>();
  auto view = TableViewBuilder(model).
    add_header_item("A0").
    add_header_item("A1").
    add_header_item("A2").
    add_header_item("A3").
    add_header_item("A4").
    add_header_item("A5").
    add_header_item("A6").
    add_header_item("A7").
    add_header_item("A8").
    add_header_item("A9").
    make();
  view->show();
  for(auto i = 0; i != 100; ++i) {
    model->push({i, i, i, i, i, i, i, i, i, i});
  }
  application->exec();
}
