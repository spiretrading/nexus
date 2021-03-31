#include <QApplication>
#include <QImageReader>
#include <QLabel>
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/ScrollBox.hpp"

using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto label = new QLabel();
  auto reader = QImageReader(":/Icons/color-picker-display.png");
  auto image = QPixmap::fromImage(reader.read());
  image = image.scaled(QSize(2000, 2000));
  label->setPixmap(std::move(image));
  auto scroll_box = new ScrollBox(label);
  scroll_box->show();
  application->exec();
}
