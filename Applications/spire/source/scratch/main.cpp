#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QRect>
#include <QColor>
#include <QGraphicsDropShadowEffect>
#include "spire/spire/resources.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/icon_button.hpp"

using namespace spire;



// ***************************************************************
// ***************************************************************
// ***************************************************************
// window
// ***************************************************************
// ***************************************************************
// ***************************************************************
class window : public QWidget {
  public:
    window(QWidget* child, bool minimize, bool maximize,
      QWidget* parent = nullptr);

    window(QWidget* child, bool minimize, bool maximize,
      const QColor& border_color, QWidget* parent = nullptr);

    QWidget* get_window();

  private:
    QWidget* m_border_widget;
    icon_button* m_icon;
    QLabel* m_title_label;
    icon_button* m_minimize_button;
    icon_button* m_maximize_button;
    icon_button* m_close_button;
    QWidget* m_child;

    void set_border_color(const QColor& color);
};

window::window(QWidget* child, bool minimize, bool maximize, QWidget* parent)
    : window(child, minimize, maximize, QColor("#A0A0A0"), parent) {}

window::window(QWidget*child, bool minimize, bool maximize,
    const QColor& border_color, QWidget* parent)
    : QWidget(parent),
      m_child(child) {
  //setWindowFlag(Qt::FramelessWindowHint);
  //setAttribute(Qt::WA_TranslucentBackground);
  setFixedSize(m_child->width() + scale_width(24),
    m_child->height() + scale_height(24));
  auto outer_layout = new QHBoxLayout(this);
  outer_layout->setSpacing(0);
  m_border_widget = new QWidget(this);
  m_border_widget->setObjectName("border");
  m_border_widget->setFixedSize(m_child->width() + 2, m_child->height() + 2);
  auto drop_shadow = new QGraphicsDropShadowEffect(this);
  drop_shadow->setBlurRadius(scale_height(120));
  drop_shadow->setXOffset(0);
  drop_shadow->setYOffset(0);
  //drop_shadow->setColor(QColor(0, 0, 0, 100));
  drop_shadow->setColor(QColor(255, 0, 0, 255));
  m_border_widget->setGraphicsEffect(drop_shadow);
  auto ss = m_child->styleSheet();
  set_border_color(border_color);
  m_child->setGraphicsEffect(nullptr);
  m_child->setStyleSheet(ss + "border: 0px solid white;");
  outer_layout->addWidget(m_border_widget);
  auto inner_layout = new QVBoxLayout(m_border_widget);
  inner_layout->setMargin(0);
  inner_layout->setSpacing(0);
  auto title_layout = new QHBoxLayout();
  inner_layout->addLayout(title_layout);
  m_icon = new icon_button(":/icons/spire-icon.svg", ":/icons/spire-icon.svg",
    scale_width(26), scale_height(20), this);
  m_icon->setFixedSize(scale_width(26), scale_height(26));
  title_layout->addWidget(m_icon);
  m_title_label = new QLabel("Window Title", this);
  m_title_label->setFixedHeight(scale_height(26));
  title_layout->addWidget(m_title_label);
  m_minimize_button = new icon_button(":/icons/minimize-grey.svg",
    ":/icons/minimize-black.svg", scale_width(32), scale_height(26), this);
  m_minimize_button->setFixedSize(scale(32, 26));
  m_minimize_button->setStyleSheet("border: none;");
  if(minimize) {
    title_layout->addWidget(m_minimize_button);
  }
  m_maximize_button = new icon_button(":/icons/maximize-grey.svg",
    ":/icons/maximize-black.svg", scale_width(32), scale_height(26), this);
  m_maximize_button->setFixedSize(scale(32, 26));
  if(maximize) {
    title_layout->addWidget(m_maximize_button);
  }
  m_close_button = new icon_button(":/icons/close-grey.svg",
    ":/icons/close-red.svg", scale_width(32), scale_height(26), this);
  m_close_button->setFixedSize(scale(32, 26));
  title_layout->addWidget(m_close_button);
  inner_layout->addWidget(m_child);
  m_child->setParent(m_border_widget);
}

QWidget* window::get_window() {
  return m_child;
}

void window::set_border_color(const QColor& color) {
  m_border_widget->setStyleSheet(QString(
    "#border { border: 1px solid %1; }").arg(color.name()));
}

// ***************************************************************
// ***************************************************************
// ***************************************************************
// test_window
// ***************************************************************
// ***************************************************************
// ***************************************************************

class test_window : public QWidget {
  public:
    test_window(int width, int height, QWidget* parent = nullptr);
};

test_window::test_window(int width, int height, QWidget* parent)
    : QWidget(parent) {
  setStyleSheet("background-color: aqua;");
}

// ***************************************************************
// ***************************************************************
// ***************************************************************
// main
// ***************************************************************
// ***************************************************************
// ***************************************************************

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto t_window = new test_window(300, 200);
  window frame(t_window, true, true);
  frame.show();
  application->exec();
}
