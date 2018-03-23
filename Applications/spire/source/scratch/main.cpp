#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QRect>
#include <QColor>
#include <QDebug>
#include <QPaintEvent>
#include <QEvent>
#include <QStyleOption>
#include <QPainter>
#include <QPushButton>
#include <QPoint>
#include <QSize>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QGraphicsDropShadowEffect>
#include "spire/spire/resources.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/icon_button.hpp"
#include "spire/ui/title_bar.hpp"

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
    window(QWidget* content, QWidget* parent = nullptr);

  protected:
    void changeEvent(QEvent* event) override;

  private:
    QWidget* m_frame_widget;
    QWidget* m_content;
    title_bar* m_title_bar;
};

::window::window(QWidget* content, QWidget* parent)
    : QWidget(parent),
      m_content(content) {
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  // append minimize button flag in case of windows,
  // for correct windows native handling of minimize function
#if defined(Q_OS_WIN)
  setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
#endif
  setAttribute(Qt::WA_TranslucentBackground);
  auto layout = new QHBoxLayout(this);
  layout->setMargin(0);
  auto drop_shadow = new QGraphicsDropShadowEffect(this);
  drop_shadow->setBlurRadius(50);
  drop_shadow->setXOffset(0);
  drop_shadow->setYOffset(0);
  drop_shadow->setColor(QColor(255, 0, 0));
  setGraphicsEffect(drop_shadow);
  m_frame_widget = new QWidget(this);
  m_frame_widget->setGraphicsEffect(drop_shadow);
  layout->addWidget(m_frame_widget);
  auto frame_layout = new QVBoxLayout(m_frame_widget);
  frame_layout->setMargin(0);
  frame_layout->setSpacing(0);
  m_title_bar = new title_bar(":/icons/spire-icon.svg",
    ":/icons/time-sale-purple.svg", this);
  frame_layout->addWidget(m_title_bar);
  frame_layout->addWidget(m_content);
}

void ::window::changeEvent(QEvent* event) {
  
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
    test_window(QWidget* parent = nullptr);
};

test_window::test_window(QWidget* parent)
    : QWidget(parent) {
  setBaseSize(600, 480);
  setStyleSheet("background-color: aqua");
  auto layout = new QHBoxLayout(this);
  auto button = new QPushButton("Button", this);
  layout->addWidget(button);
  window()->setWindowTitle("Test Title");
  static auto num = 0;
  connect(button, &QPushButton::clicked, [=] {
    window()->setWindowTitle(window()->windowTitle() + QString("%1").arg(++num));
  });
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



  auto content = new test_window();

  ::window frame(content);
  frame.show();



  application->exec();
}


//#include <QApplication>
//#include <QWidget>
//#include <QHBoxLayout>
//#include <QVBoxLayout>
//#include <QLabel>
//#include <QRect>
//#include <QColor>
//#include <QDebug>
//#include <QPaintEvent>
//#include <QEvent>
//#include <QStyleOption>
//#include <QPainter>
//#include <QPushButton>
//#include <QPoint>
//#include <QSize>
//#include <QDesktopWidget>
//#include <QGraphicsDropShadowEffect>
//#include "spire/spire/resources.hpp"
//#include "spire/spire/dimensions.hpp"
//#include "spire/ui/icon_button.hpp"
//
//using namespace spire;
//
//
//
//// ***************************************************************
//// ***************************************************************
//// ***************************************************************
//// window
//// ***************************************************************
//// ***************************************************************
//// ***************************************************************
//class window : public QWidget {
//  public:
//    window(QWidget* child, bool minimize, bool maximize,
//      QWidget* parent = nullptr);
//
//    window(QWidget* child, bool minimize, bool maximize,
//      const QColor& border_color, QWidget* parent = nullptr);
//
//    QWidget* get_window();
//
//  protected:
//      void changeEvent(QEvent* event);
//      bool event(QEvent* event);
//      void mouseMoveEvent(QMouseEvent* event) override;
//      void mousePressEvent(QMouseEvent* event) override;
//      void mouseReleaseEvent(QMouseEvent* event) override;
//
//  private:
//    QWidget* m_border_widget;
//    icon_button* m_icon;
//    QLabel* m_title_label;
//    icon_button* m_minimize_button;
//    icon_button* m_maximize_button;
//    icon_button* m_restore_button;
//    icon_button* m_close_button;
//    QWidget* m_child;
//    bool m_is_dragging;
//    QPoint m_last_pos;
//    QSize m_last_window_size;
//    QGraphicsDropShadowEffect* m_drop_shadow;
//    QGraphicsDropShadowEffect* m_null_drop_shadow;
//
//    void minimize_button_pressed();
//    void maximize_button_pressed();
//    void restore_button_pressed();
//    void set_border(const QColor& color);
//};
//
//window::window(QWidget* child, bool minimize, bool maximize, QWidget* parent)
//    : window(child, minimize, maximize, QColor("#A0A0A0"), parent) {}
//
//window::window(QWidget*child, bool minimize, bool maximize,
//    const QColor& border_color, QWidget* parent)
//    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint),
//      m_child(child),
//      m_is_dragging(false) {
//  setAttribute(Qt::WA_TranslucentBackground);
//  auto outer_layout = new QHBoxLayout(this);
//  outer_layout->setSpacing(0);
//  m_border_widget = new QWidget(this);
//  if(!maximize) {
//    setFixedSize(m_child->width() + scale_width(24),
//      m_child->height() + scale_height(24 + 26));
//
//    m_border_widget->setFixedSize(m_child->width() + 2,
//      m_child->height() + scale_height(26) + 1);
//  } else {
//    setBaseSize(m_child->width() + scale_width(24),
//      m_child->height() + scale_height(24 + 26));
//  }
//  m_null_drop_shadow = new QGraphicsDropShadowEffect(this);
//  m_drop_shadow = new QGraphicsDropShadowEffect(this);
//  m_drop_shadow->setBlurRadius(scale_height(12));
//  m_drop_shadow->setXOffset(0);
//  m_drop_shadow->setYOffset(0);
//  m_drop_shadow->setColor(QColor(0, 0, 0, 100));
//  m_border_widget->setGraphicsEffect(m_drop_shadow);
//  auto ss = m_child->styleSheet();
//  set_border(border_color);
//  m_child->setGraphicsEffect(nullptr);
//  m_child->setStyleSheet(ss + "border: none;");
//  outer_layout->addWidget(m_border_widget);
//  auto inner_layout = new QVBoxLayout(m_border_widget);
//  inner_layout->setMargin(0);
//  inner_layout->setSpacing(0);
//  auto title_layout = new QHBoxLayout();
//  inner_layout->addLayout(title_layout);
//  m_icon = new icon_button(":/icons/spire-icon.svg", ":/icons/bookview-black.svg",
//    scale_width(26), scale_height(26),
//    QRect(scale_width(8), scale_height(8), scale_width(10), scale_height(10)),
//    this);
//  m_icon->hover_active(false);
//  m_icon->setFixedSize(scale_width(26), scale_height(26));
//  m_icon->setStyleSheet(R"(
//    background-color: #F5F5F5;
//    border-bottom: none;
//    border-right: none;)");
//  title_layout->addWidget(m_icon);
//  m_title_label = new QLabel("Window Title", this);
//  m_title_label->setFixedHeight(scale_height(26));
//  m_title_label->setStyleSheet(QString(R"(
//    background-color: #F5F5F5;
//    border-bottom: none;
//    border-left: none;
//    border-right: none;
//    font-family: Roboto;
//    font-size: %1px;)").arg(scale_height(12)));
//  title_layout->addWidget(m_title_label);
//  m_minimize_button = new icon_button(":/icons/minimize-grey.svg",
//    ":/icons/minimize-black.svg", scale_width(32), scale_height(26),
//    QRect(scale_width(11), scale_height(12), scale_width(10), scale_height(2)),
//    this);
//  m_minimize_button->connect_clicked_signal([&] { minimize_button_pressed(); });
//  m_minimize_button->setFixedSize(scale(32, 26));
//  m_minimize_button->setStyleSheet(R"(
//    QWidget { background-color: #F5F5F5;
//              border-bottom: none;
//              border-left: none;
//              border-right: none;
//    }
//    :hover { background-color: #EBEBEB; })");
//  if(minimize) {
//    title_layout->addWidget(m_minimize_button);
//  }
//  m_maximize_button = new icon_button(":/icons/maximize-grey.svg",
//    ":/icons/maximize-black.svg", scale_width(32), scale_height(26),
//    QRect(scale_width(11), scale_height(8), scale_width(10), scale_height(10)),
//    this);
//  m_maximize_button->connect_clicked_signal([=] { maximize_button_pressed(); });
//  m_maximize_button->setFixedSize(scale(32, 26));
//  m_maximize_button->setStyleSheet(R"(
//    QWidget { background-color: #F5F5F5;
//              border-bottom: none;
//              border-left: none;
//              border-right: none;
//    }
//    :hover { background-color: #EBEBEB; })");
//  if(maximize) {
//    title_layout->addWidget(m_maximize_button);
//  }
//  m_restore_button = new icon_button(":/icons/unmaximize-grey.svg",
//    ":/icons/unmaximize-black.svg", scale_width(32), scale_height(26),
//    QRect(scale_width(11), scale_height(8), scale_width(10), scale_height(10)),
//    this);
//  m_restore_button->connect_clicked_signal([=] { restore_button_pressed(); });
//  m_restore_button->setFixedSize(scale(32, 26));
//  m_restore_button->setStyleSheet(R"(
//    QWidget { background-color: #F5F5F5;
//              border-bottom: none;
//              border-left: none;
//              border-right: none;
//    }
//    :hover { background-color: #EBEBEB; })");
//  if(maximize) {
//    title_layout->addWidget(m_restore_button);
//    m_restore_button->setVisible(false);
//  }
//  m_close_button = new icon_button(":/icons/close-grey.svg",
//    ":/icons/close-red.svg", scale_width(32), scale_height(26),
//    QRect(scale_width(11), scale_height(8), scale_width(10), scale_height(10)),
//    this);
//  m_close_button->connect_clicked_signal([&] { close(); });
//  m_close_button->setFixedSize(scale(32, 26));
//  m_close_button->setStyleSheet(R"(
//    QWidget { background-color: #F5F5F5;
//              border-bottom: none;
//              border-left: none;
//    }
//    :hover { background-color: #EBEBEB; })");
//  title_layout->addWidget(m_close_button);
//  auto content_layout = new QHBoxLayout();
//  content_layout->setMargin(0);
//  inner_layout->addLayout(content_layout);
//  content_layout->addWidget(m_child);
//  m_child->setParent(m_border_widget);
//}
//
//QWidget* window::get_window() {
//  return m_child;
//}
//
//void window::changeEvent(QEvent* event) {
//  if(event->type() == QEvent::WindowStateChange) {
//    if(windowState().testFlag(Qt::WindowMaximized)) {
//      layout()->setMargin(0);
//      setGraphicsEffect(nullptr);
////      m_restore_button->setVisible(true);
////      m_maximize_button->setVisible(false);
//      update();
//    } else if(windowState().testFlag(Qt::WindowNoState)) {
//      layout()->setMargin(10);
//      setGraphicsEffect(m_drop_shadow);
////      m_restore_button->setVisible(false);
////      m_maximize_button->setVisible(true);
//      update();
//    }
//  }
//  QWidget::changeEvent(event);
//}
//
//void window::minimize_button_pressed() {
//  setWindowState(Qt::WindowMinimized);
//}
//
//void window::maximize_button_pressed() {
//  if(windowState().testFlag(Qt::WindowMaximized)) {
//    setWindowState(Qt::WindowNoState);
//  } else {
//    setWindowState(Qt::WindowMaximized);
//  }
//}
//
//void window::restore_button_pressed() {
//  m_restore_button->setVisible(false);
//  m_maximize_button->setVisible(true);
//  setWindowState(Qt::WindowNoState);
//}
//
//bool window::event(QEvent* event) {
//  if(event->type() == QEvent::WindowActivate) {
//    m_title_label->setStyleSheet(QString(
//      R"(background-color: #F5F5F5;
//         border-bottom: none;
//         border-left: none;
//         border-right: bottom;
//         font-family: Roboto;
//         font-size: %1px;)").arg(scale_height(12)));
//    m_icon->set_icon(true);
//  } else if(event->type() == QEvent::WindowDeactivate) {
//    m_title_label->setStyleSheet(m_title_label->styleSheet() +
//      "color: rgba(0, 0, 0, 102);");
//    m_icon->set_icon(false);
//  }
//  return QWidget::event(event);
//}
//
//void window::mouseMoveEvent(QMouseEvent* event) {
//  if(!m_is_dragging) {
//    return;
//  }
//  auto delta = event->globalPos();
//  delta -= m_last_pos;
//  auto window_pos = pos();
//  window_pos += delta;
//  m_last_pos = event->globalPos();
//  move(window_pos);
//}
//
//void window::mousePressEvent(QMouseEvent* event)  {
//  if(m_is_dragging || event->button() != Qt::LeftButton) {
//    return;
//  }
//  // Check if the mouse is within the title bar
//  m_is_dragging = true;
//  m_last_pos = event->globalPos();
//}
//void window::mouseReleaseEvent(QMouseEvent* event) {
//  if(event->button() != Qt::LeftButton) {
//    return;
//  }
//  m_is_dragging = false;
//}
//
//void window::set_border(const QColor& color) {
//  m_border_widget->setStyleSheet(QString(
//    "border: 1px solid %1;").arg(color.name()));
//}
//
//// ***************************************************************
//// ***************************************************************
//// ***************************************************************
//// test_window
//// ***************************************************************
//// ***************************************************************
//// ***************************************************************
//
//class test_window : public QWidget {
//  public:
//    test_window(QWidget* parent = nullptr);
//};
//
//test_window::test_window(QWidget* parent)
//    : QWidget(parent) {
//  //setBaseSize(scale(600, 600));
//  setStyleSheet("background-color: aqua;");
//  auto layout = new QVBoxLayout(this);
//  auto b1 = new QPushButton("button1", this);
//  b1->setStyleSheet("background-color: red");
//  layout->addWidget(b1);
//}
//
//// ***************************************************************
//// ***************************************************************
//// ***************************************************************
//// main
//// ***************************************************************
//// ***************************************************************
//// ***************************************************************
//
//int main(int argc, char** argv) {
//  auto application = new QApplication(argc, argv);
//  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
//  application->setApplicationName(QObject::tr("Scratch"));
//  initialize_resources();
//  auto t_window = new test_window();
//  window frame(t_window, true, true);
//  frame.show();
//  application->exec();
//}
