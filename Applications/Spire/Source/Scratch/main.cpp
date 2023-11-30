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
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  application.exec();
}
