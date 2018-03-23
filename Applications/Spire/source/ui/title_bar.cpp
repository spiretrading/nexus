#include "spire/ui/title_bar.hpp"
#include <QApplication>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/icon_button.hpp"






#include <QDebug>




using namespace spire;

title_bar::title_bar(const QString& icon, const QString& unfocused_icon,
    QWidget* parent)
    : QWidget(parent) {
  connect(window(), &QWidget::windowTitleChanged,
    [=] { on_window_title_change(); });
  setFixedHeight(scale_height(26));
  setStyleSheet("background-color: #F5F5F5;");
  auto layout = new QHBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(0);
  m_icon = new icon_button(icon, unfocused_icon,
    scale_width(26), scale_height(26),
    QRect(scale_width(8), scale_height(8), scale_width(10), scale_height(10)),
    this);
  m_icon->setFixedWidth(scale_width(26));
  m_icon->hover_active(false);
  layout->addWidget(m_icon);
  m_title_label = new QLabel(tr("Title Text"), this);
  m_title_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(m_title_label);
  m_minimize_button = new icon_button(":/icons/minimize-grey.svg",
    ":/icons/minimize-black.svg", scale_width(32), scale_height(26),
    QRect(scale_width(11), scale_height(12), scale_width(10), scale_height(2)),
    this);
  m_minimize_button->connect_clicked_signal(
    [=] { on_minimize_button_press(); });
  m_minimize_button->setFixedWidth(scale_width(32));
  m_minimize_button->setStyleSheet(R"(
    QWidget { background-color: #F5F5F5; }
    :hover { background-color: #EBEBEB; })");
  layout->addWidget(m_minimize_button);
  m_maximize_button = new icon_button(":/icons/maximize-grey.svg",
    ":/icons/maximize-black.svg", scale_width(32), scale_height(26),
    QRect(scale_width(11), scale_height(8), scale_width(10), scale_height(10)),
    this);
  m_maximize_button->connect_clicked_signal(
    [=] { on_maximize_button_press(); });
  m_maximize_button->setFixedWidth(scale_width(32));
  m_maximize_button->setStyleSheet(R"(
    QWidget { background-color: #F5F5F5; }
    :hover { background-color: #EBEBEB; })");
  layout->addWidget(m_maximize_button);
  m_restore_button = new icon_button(":/icons/unmaximize-grey.svg",
    ":/icons/unmaximize-black.svg", scale_width(32), scale_height(26),
    QRect(scale_width(11), scale_height(8), scale_width(10), scale_height(10)),
    this);
  m_restore_button->connect_clicked_signal([=] { on_restore_button_press(); });
  m_restore_button->setVisible(false);
  m_restore_button->setFixedWidth(scale_width(32));
  m_restore_button->setStyleSheet(R"(
    QWidget { background-color: #F5F5F5; }
    :hover { background-color: #EBEBEB; })");
  layout->addWidget(m_restore_button);
  m_close_button = new icon_button(":/icons/close-grey.svg",
    ":/icons/close-red.svg", scale_width(32), scale_height(26),
    QRect(scale_width(11), scale_height(8), scale_width(10), scale_height(10)),
    this);
  m_close_button->connect_clicked_signal([=] { on_close_button_press(); });
  m_close_button->setFixedWidth(scale_width(32));
  m_close_button->setStyleSheet(R"(
    QWidget { background-color: #F5F5F5;}
    :hover { background-color: #EBEBEB; })");
  layout->addWidget(m_close_button);
}

void title_bar::mouseDoubleClickEvent(QMouseEvent* event) {
  if(window()->geometry() == QApplication::desktop()->availableGeometry(
      QApplication::desktop()->screenNumber(this))) {
    on_restore_button_press();
  } else {
    on_maximize_button_press();
  }
}

void title_bar::on_window_title_change() {
  m_title_label->setText(window()->windowTitle());
}

void title_bar::on_minimize_button_press() {
  window()->setWindowState(Qt::WindowMinimized);
}

void title_bar::on_maximize_button_press() {
  m_previous_geometry = window()->geometry();
  window()->setGeometry(QApplication::desktop()->availableGeometry(
    QApplication::desktop()->screenNumber(this)));
  m_maximize_button->setVisible(false);
  m_restore_button->setVisible(true);
  //auto e = new QEvent(QEvent::Leave);
  //QCoreApplication::postEvent(m_maximize_button, e);
  //e = new QEvent(QEvent::Leave);
  //QCoreApplication::postEvent(m_restore_button, e);
}

void title_bar::on_restore_button_press() {
  // TODO: assign some initial value to m_previous_geometry in the constructor,
  //       just in case a window starts out maximized
  window()->setGeometry(m_previous_geometry);
  m_maximize_button->setVisible(true);
  m_restore_button->setVisible(false);
  //auto e = new QEvent(QEvent::Leave);
  //QCoreApplication::postEvent(m_maximize_button, e);
  //e = new QEvent(QEvent::Leave);
  //QCoreApplication::postEvent(m_restore_button, e);
}

void title_bar::on_close_button_press() {
  window()->close();
}
