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
#include "Spire/Ui/Box.hpp"
#include <QLabel>

namespace {
  double get_transition_point(int border_width1, int border_width2) {
    return static_cast<double>(border_width1) / (border_width1 + border_width2);
  }

  QPointF get_curve_pos(
      QPointF start, QPointF ctrl, QPointF end, double percent) {
    float x1 = (ctrl.x() - start.x()) * percent + start.x();
    float y1 = (ctrl.y() - start.y()) * percent + start.y();
    float x2 = (end.x() - ctrl.x()) * percent + ctrl.x();
    float y2 = (end.y() - ctrl.y()) * percent + ctrl.y();
    return {(x2 - x1) * percent + x1, (y2 - y1) * percent + y1};
  }

  // TODO: maybe update with and height to be the final size of the path,
  //        not the size of the widget.
  // assumes top side, which can be rotated into place
  // builds the path geometry without color
  QPainterPath create_border_side(int left_radius, int right_radius,
      int border_width, int width,
      int height, int previous_border_width, int next_border_width) {
    // TODO: what if border_width is 0?

    // TODO: investigate potential off-by-ones with width and height

    // TODO: investigate if the precense of the left/right borders matter,
    //        it may be the case that 'sibling-less' borders will function
    //        properly with the same calculation that would be applied
    //        to a border that shares a corner.

    // **** LOC ****
    auto loc_start = [&] () -> QPointF {
      if(previous_border_width <= 0) {
        return QPointF(0, left_radius);
      }
      // else, does share a left border
      auto start = QPointF(0, left_radius);
      auto end = QPointF(left_radius, 0);
      auto point = get_curve_pos(start, QPointF(0, 0), end,
        get_transition_point(border_width, previous_border_width));
      return point;
    }();
    auto loc_end = [&] () -> QPointF {
      // Note: LOC end is indepdendent of shared border.
      //if(next_border_width <= 0) {
        return QPointF(left_radius, 0);
      //}
      // else, does share a left border
      //auto start = QPointF(0, left_radius);
      //auto end = QPointF();
      //auto ratio = static_cast<double>(previous_border_width) / border_width;
      //return get_curve_pos(start, QPointF(0, 0), end, ratio);
    }();
    
    // **** ROC ****
    auto roc_start = [&] () -> QPointF {
      // Note: independent of shared border
      //if(next_border_width <= 0) {
        return QPointF(width - right_radius, 0);
      //}
      // shares border with right side
      //return QPointF();
    }();
    auto roc_end = [&] () -> QPointF {
      if(next_border_width <= 0) {
        return QPointF(width, right_radius);
      }
      // shares border with right side
      auto start = QPointF(width - right_radius, 0);
      auto end = QPointF(width, right_radius);
      auto tp = get_transition_point(border_width, next_border_width);
      auto point = get_curve_pos(start, QPointF(width, 0), end, tp);
      return point;
    }();

    // **** RIC ****
    // Note: lines get reversed here, and the start is now on the right side of the end
    auto ric_start = [&] () -> QPointF {
      if(next_border_width <= 0) {
        return QPointF(width, right_radius);
      }
      // shares border with right side
      auto start = QPointF(width - next_border_width, right_radius);
      auto end = QPointF(width - right_radius, border_width);
      auto tp = get_transition_point(border_width, next_border_width);
      auto point = get_curve_pos(
        start, QPointF(width - next_border_width, border_width), end, tp);
      return point;
    }();
    auto ric_ctrl = [&] () -> QPointF {
      return QPointF(width - next_border_width, border_width);
    }();
    auto ric_end = [&] () -> QPointF {
      if(next_border_width <= 0) {
        // TODO: may not be right, may need to adjust (radius - width), see docs
        return QPointF(width - right_radius, border_width);
      }
      // shares border with right side
      auto start = QPointF(width - next_border_width, right_radius);
      auto end = QPointF(width - right_radius, border_width);
      auto point = get_curve_pos(
        end, QPointF(width - next_border_width, border_width), start,
        get_transition_point(border_width, next_border_width));
      return point;
    }();

    // **** LIC ****
    auto lic_start = [&] () -> QPointF {
      if(previous_border_width <= 0) {
        return QPointF(previous_border_width + left_radius, border_width);
      }
      // shares border with left side
      // TODO: these curves are the same for each start/end of the inner corners,
      //        so don't duplicate
      // TODO: constrain these radius - border calculations
      auto start = QPointF(
        previous_border_width + (left_radius - previous_border_width),
        border_width);
      auto end = QPointF(previous_border_width,
        border_width + (left_radius - border_width));
      auto tp = get_transition_point(border_width, next_border_width);
      auto point = get_curve_pos(
        start, QPointF(previous_border_width, border_width), end, tp);
      return point;
    }();
    auto lic_ctrl = [&] () -> QPointF {
      return QPointF(previous_border_width, border_width);
    }();
    auto lic_end = [&] () -> QPointF {
      if(previous_border_width <= 0) {
        return QPointF(previous_border_width, border_width + (left_radius - border_width));
      }
      // shares border with left side
      auto start = QPointF(
        previous_border_width + (left_radius - border_width),
        border_width);
      auto end = QPointF(previous_border_width,
        border_width + (left_radius - border_width));
      auto tp = get_transition_point(border_width, next_border_width);
      auto point = get_curve_pos(
        start, QPointF(previous_border_width, border_width), end, tp);
      return point;
    }();
    
    //loc_start = QPointF(17, 17);
    //loc_end = QPointF(57, 0);
    //roc_start = QPointF(144, 0);
    //roc_end = QPointF(182, 17);
    //ric_start = QPointF(154, 46);
    //ric_end = QPointF(140, 40);
    //lic_start = QPointF(60, 40);
    //lic_end = QPointF(46, 46);

    //ric_end = {140, 40};
    //lic_start = {60, 40};

    // TODO: the control points don't account for the corner being split,
    //        so they're still set as if it was the full radius.

    auto path = QPainterPath(loc_start);
    // fixed left outer control point
    path.quadTo(QPointF(0, 0), loc_end);
    path.lineTo(roc_start);
    // fixed right outer control point
    path.quadTo(QPointF(width, 0), roc_end);
    path.lineTo(ric_start);
    path.quadTo(ric_ctrl, ric_end);
    path.lineTo(lic_start);
    path.quadTo(lic_ctrl, lic_end);
    // close the path
    path.lineTo(loc_start);

    return path;
  }
}

class PaintTest : public QWidget {
  protected:
    void paintEvent(QPaintEvent* event) override {
      auto top_width = 40;
      auto right_width = 40;
      auto bottom_width = 40;
      auto left_width = 40;
      // Note: these need to be manually reduced!
      auto tl_radius = 60;
      auto tr_radius = 60;
      auto br_radius = 60;
      auto bl_radius = 60;
      auto painter = QPainter(this);
      painter.fillRect(0, 0, width(), height(), QColor(0, 255, 255));
      painter.setPen(Qt::NoPen);
      // TODO: render onto QImage/QPixmap because otherwise the background of the parent
      //        is blended, too, or find another method
      //painter.setCompositionMode(QPainter::CompositionMode_Plus);
      painter.setRenderHint(QPainter::HighQualityAntialiasing);

      painter.setBrush(QColor(Qt::red));
      painter.drawPath(create_border_side(tl_radius, tr_radius, top_width,
        width(), height(), left_width, right_width));

      painter.rotate(90);
      painter.translate(0, -width());
      painter.setBrush(QColor(Qt::green));
      painter.drawPath(create_border_side(tr_radius, br_radius, right_width,
        height(), width(), top_width, bottom_width));

      painter.rotate(90);
      painter.translate(0, -height());
      painter.setBrush(QColor(Qt::blue));
      painter.drawPath(create_border_side(br_radius, bl_radius, bottom_width,
        width(), height(), right_width, left_width));

      painter.rotate(90);
      painter.translate(0, -width());
      painter.setBrush(QColor(Qt::yellow));
      painter.drawPath(create_border_side(bl_radius, tr_radius, left_width,
        height(), width(), bottom_width, top_width));
    }
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = new QWidget();
  w->resize(350, 350);
  //w->setAttribute(Qt::WA_TranslucentBackground);
  auto l = new QHBoxLayout(w);
  auto pt = new PaintTest();
  pt->setFixedSize(200, 200);
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
  nw->setFixedSize(200, 200);
  l->addWidget(nw);
  //auto b = make_input_box(nullptr, w);
  //b->setFixedSize(100, 100);
  //l->addWidget(b);
  w->show();
  application->exec();
}
