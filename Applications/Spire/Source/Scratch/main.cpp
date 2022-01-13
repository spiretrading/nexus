#include <QApplication>
#include <QPainter>
#include <QWidget>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/BoxPainter.hpp"

using namespace Spire;

struct Canvas : QWidget {
  Canvas()
    : m_box_painter(Qt::black, {{scale_height(10), QColor(0xFF0000)},
        {scale_width(10), QColor(0x00FF00)},
        {scale_height(10), QColor(0xFF00FF)},
        {scale_width(20), QColor(0x0000FF)}, scale_width(10),
        scale_width(5), scale_width(70), scale_width(10)}) {
  }

  void paintEvent(QPaintEvent* event) override {
    auto painter = QPainter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    m_box_painter.paint(painter);
  };

  BoxPainter m_box_painter;
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto canvas = new Canvas();
  canvas->resize(scale(150, 150));
  canvas->show();
  application->exec();
}
