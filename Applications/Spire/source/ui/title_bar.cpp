#include "spire/ui/title_bar.hpp"
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
  m_minimize_button->setFixedWidth(scale_width(32));
  m_minimize_button->setStyleSheet(R"(
    QWidget { background-color: #F5F5F5; }
    :hover { background-color: #EBEBEB; })");
  layout->addWidget(m_minimize_button);
  m_maximize_button = new icon_button(":/icons/maximize-grey.svg",
    ":/icons/maximize-black.svg", scale_width(32), scale_height(26),
    QRect(scale_width(11), scale_height(8), scale_width(10), scale_height(10)),
    this);
  //m_maximize_button->connect_clicked_signal([=] { window()->setGe });
  m_maximize_button->setFixedWidth(scale_width(32));
  m_maximize_button->setStyleSheet(R"(
    QWidget { background-color: #F5F5F5; }
    :hover { background-color: #EBEBEB; })");
  layout->addWidget(m_maximize_button);
  m_restore_button = new icon_button(":/icons/unmaximize-grey.svg",
    ":/icons/unmaximize-black.svg", scale_width(32), scale_height(26),
    QRect(scale_width(11), scale_height(8), scale_width(10), scale_height(10)),
    this);
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
  m_close_button->setFixedSize(scale(32, 26));
  m_close_button->setStyleSheet(R"(
    QWidget { background-color: #F5F5F5;}
    :hover { background-color: #EBEBEB; })");
  layout->addWidget(m_close_button);
}

void title_bar::on_window_title_change() {
  qDebug() << "Changing window title...";
  m_title_label->setText(window()->windowTitle());
}

void title_bar::on_minimize_button_press() {

}

void title_bar::on_maximize_button_press() {

}

void title_bar::on_close_button_press() {

}
