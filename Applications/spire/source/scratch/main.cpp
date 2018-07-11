#include <QApplication>
#include "spire/spire/resources.hpp"
#include "spire/ui/flat_button.hpp"

using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto button = new flat_button("Test Button");
  flat_button::style s;
  s.m_background_color = QColor("#0000FF");
  s.m_text_color = Qt::black;
  button->set_style(s);
  button->resize(400, 250);
  button->show();
  application->exec();
}
