#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QBitmap>
#include <QLinearGradient>
#include <QPaintEngine>
#include <QWidget>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>

class PaintTest : public QWidget {
  protected:
    void paintEvent(QPaintEvent* event) override {
      auto border_width = 10;
      auto radius = 20;
      auto painter = QPainter(this);
      painter.setPen(Qt::NoPen);
      // TODO: render onto QImage/QPixmap because otherwise the background of the parent
      //        is blended, too
      //painter.setCompositionMode(QPainter::CompositionMode_Plus);
      //painter.setRenderHint(QPainter::HighQualityAntialiasing);
      auto top = QPainterPath(QPointF(0, radius));
      top.quadTo(0, 0, radius, 0);
      // TODO: fix off-by-ones with width and height
      top.lineTo(width() - radius, 0);
      top.quadTo(width(), 0, width(), radius);
      top.quadTo(width() - border_width, border_width, width() - radius, border_width);
      top.lineTo(radius, border_width);
      top.quadTo(border_width, border_width, 0, radius);
      //painter.setPen(Qt::red);
      painter.setBrush(QColor(Qt::red));
      painter.drawPath(top);

      painter.rotate(90);
      painter.translate(0, -width());
      painter.setBrush(QColor(Qt::green));
      painter.drawPath(top);

      painter.rotate(90);
      painter.translate(0, -height());
      painter.setBrush(QColor(Qt::blue));
      painter.drawPath(top);

      painter.rotate(90);
      painter.translate(0, -width());
      painter.setBrush(QColor(Qt::yellow));
      painter.drawPath(top);
    }
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = new QWidget();
  w->resize(250, 250);
  auto l = new QHBoxLayout(w);
  auto pt = new PaintTest();
  pt->setFixedSize(100, 100);
  l->addWidget(pt);
  auto nw = new QWidget();
  nw->setStyleSheet(QString(R"(
    border: 20px solid red;
    border-radius: 25px;
    border-top-color: red;
    border-right-color: green;
    /*border-right: none;*/
    border-bottom-color: blue;
    border-left-color: yellow;
  )"));
  nw->setFixedSize(100, 100);
  l->addWidget(nw);
  w->show();
  application->exec();
}
