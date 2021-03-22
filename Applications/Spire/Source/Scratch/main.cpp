#include <QApplication>
#include <QHBoxLayout>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/ScrollBox.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto widget = new QWidget();
  auto layout = new QHBoxLayout(widget);
  layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Policy::Expanding,
    QSizePolicy::Policy::Expanding));
  layout->addWidget(new ScrollBar(ScrollBar::Orientation::VERTICAL));
  widget->show();
  widget->resize(800, 800);
  application->exec();
}
