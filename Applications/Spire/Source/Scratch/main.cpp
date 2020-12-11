#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QDebug>
#include <QLabel>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QStyleOption>
#include <QProxyStyle>

#include "Spire/Spire/Dimensions.hpp"

class ScrollBarStyle : public QProxyStyle {
  public:

    // TODO: lifetime?
    ScrollBarStyle(QStyle* style = nullptr, QWidget* parent = nullptr)
      : QProxyStyle(style),
        m_horizontal_handle_height(scale_height(13)),
        m_minimum_horizontal_handle_width(scale_width(60)),
        m_vertical_handle_width(scale_width(13)),
        m_minimum_vertical_handle_height(scale_height(60)) {
      //setParent(parent);
    }

    // TODO: setters for handle widths.

    void drawComplexControl(QStyle::ComplexControl cc,
        const QStyleOptionComplex *opt,
        QPainter *p, const QWidget *widget) const override {
      if(cc == CC_ScrollBar) {
        p->fillRect(opt->rect, Qt::white);
        auto slider_option = QStyleOptionSlider();
        slider_option.rect = subControlRect(CC_ScrollBar, &slider_option,
          SC_ScrollBarSlider, widget);
        drawControl(CE_ScrollBarSlider, &slider_option, p, widget);
        return;
      }
      QProxyStyle::drawComplexControl(cc, opt, p, widget);
    }

    // TODO: widget = nullptr for these methods?
    void drawControl(QStyle::ControlElement element,
        const QStyleOption *option, QPainter *painter,
        const QWidget *widget) const override {
      if(element == CE_ScrollBarSlider) {
        painter->fillRect(option->rect, QColor("#C8C8C8"));
        return;
      }
      QProxyStyle::drawControl(element, option, painter, widget);
    }

    int pixelMetric(PixelMetric metric, const QStyleOption* opt,
        const QWidget* widget) const override {
      if(metric == PM_ScrollBarExtent) {
        auto scroll_bar = qobject_cast<const QScrollBar*>(widget);
        if(scroll_bar->orientation() == Qt::Horizontal) {
          return scale_height(13);
        } else {
          return scale_width(13);
        }
      }
      return QProxyStyle::pixelMetric(metric, opt, widget);
    }

    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
        SubControl sc, const QWidget *widget) const override {
      switch(sc) {
        case SC_ScrollBarAddLine:
        case SC_ScrollBarAddPage:
        case SC_ScrollBarFirst:
        case SC_ScrollBarLast:
        case SC_ScrollBarSubLine:
        case SC_ScrollBarSubPage:
          return {};
        case SC_ScrollBarGroove:
          return widget->rect();
        case SC_ScrollBarSlider:
          {
            auto scroll_bar = qobject_cast<const QScrollBar*>(widget);
            if(scroll_bar->orientation() == Qt::Horizontal) {
              auto slider_pos = sliderPositionFromValue(scroll_bar->minimum(),
                scroll_bar->maximum(), scroll_bar->value(),
                scroll_bar->width() - m_minimum_horizontal_handle_width);
              return QRect(slider_pos, 0, m_minimum_horizontal_handle_width,
                m_horizontal_handle_height);
            } else {
              auto slider_pos = sliderPositionFromValue(scroll_bar->minimum(),
                scroll_bar->maximum(), scroll_bar->value(),
                scroll_bar->height() - m_minimum_vertical_handle_height);
              return QRect(0, slider_pos, m_vertical_handle_width,
                m_minimum_vertical_handle_height);
            }
          }
      }
      return QProxyStyle::subControlRect(cc, opt, sc, widget);
    }

  private:
    int m_horizontal_handle_height;
    int m_minimum_horizontal_handle_width;
    int m_vertical_handle_width;
    int m_minimum_vertical_handle_height;
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  initialize_resources();
  auto w = new QScrollArea();
  w->setFrameShape(QFrame::NoFrame);
  w->setCornerWidget(nullptr);
  w->verticalScrollBar()->setStyle(
    new ScrollBarStyle(w->verticalScrollBar()->style(), w));
  w->horizontalScrollBar()->setStyle(
    new ScrollBarStyle(w->horizontalScrollBar()->style(), w));
  auto l = new QLabel("Test Label", w);
  l->setStyleSheet(R"(
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 red, stop:1 blue);
    font-size: 100px;)");
  l->setFixedSize(10000, 10000);
  w->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  w->verticalScrollBar()->setFixedWidth(13);
  w->setWidget(l);
  //w->verticalScrollBar()->setStyle(new SBStyle(w->verticalScrollBar()->style()));
  w->resize(400, 300);
  w->show();
  return application->exec();
}
