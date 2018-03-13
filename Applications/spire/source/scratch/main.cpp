#include <QApplication>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QLabel>
#include <QMovie>
#include "spire/spire/resources.hpp"

using namespace spire;




#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QString>
#include <QVBoxlayout>
#include <QWidget>

#include "spire/spire/dimensions.hpp"




int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();

  auto toolbar = new QWidget();
  toolbar->setWindowFlag(Qt::FramelessWindowHint);
  toolbar->setContentsMargins(0, 0, 0, 0);
  toolbar->setFixedSize(scale(308, 98));
  toolbar->setStyleSheet("background-color: #F5F5F5;");
  auto layout = new QVBoxLayout(toolbar);
  layout->setMargin(0);
  layout->setSpacing(0);

  auto title_bar_layout = new QHBoxLayout();
  title_bar_layout->setMargin(0);
  title_bar_layout->setSpacing(0);
  layout->addLayout(title_bar_layout);
  auto title_bar_logo = new QLabel("@", toolbar);
  title_bar_logo->setStyleSheet("qproperty-alignment: AlignCenter;");
  title_bar_logo->setFixedSize(scale(26, 26));
  title_bar_layout->addWidget(title_bar_logo);
  auto username_label = new QLabel("Spire - Signed in, Username");
  username_label->setFixedSize(scale(218, 26));
  title_bar_layout->addWidget(username_label);
  auto minimize_button = new QLabel("-", toolbar);
  minimize_button->setStyleSheet("qproperty-alignment: AlignCenter;");
  minimize_button->setFixedSize(scale(32, 26));
  title_bar_layout->addWidget(minimize_button);
  auto close_button = new QLabel("x", toolbar);
  close_button->setStyleSheet("qproperty-alignment: AlignCenter;");
  close_button->setFixedSize(scale(32, 26));
  title_bar_layout->addWidget(close_button);
  


  auto input_layout = new QVBoxLayout();
  input_layout->setMargin(0);
  input_layout->setSpacing(0);
  layout->addLayout(input_layout);
  auto combobox_layout = new QHBoxLayout();
  combobox_layout->setContentsMargins(scale_width(8), scale_height(8), scale_width(8), scale_height(5));
  combobox_layout->setSpacing(0);
  input_layout->addLayout(combobox_layout);
  auto window_manager_combobox = new QComboBox(toolbar);
  window_manager_combobox->setFixedSize(scale(138, 26));
  window_manager_combobox->setStyleSheet(R"(
    background-color: white;
    border: 1px solid #C8C8C8;)");
  window_manager_combobox->addItem("Window Manager");
  window_manager_combobox->addItem(QObject::tr("Minimize All"));
  window_manager_combobox->addItem(QObject::tr("Restore All"));
  window_manager_combobox->addItem(QObject::tr("Settings"));
  combobox_layout->addWidget(window_manager_combobox);
  combobox_layout->addStretch(1);
  auto recently_closed_combobox = new QComboBox(toolbar);
  recently_closed_combobox->setFixedSize(scale(138, 26));
  recently_closed_combobox->setStyleSheet(R"(
    background-color: white;
    border: 1px solid #C8C8C8;)");
  recently_closed_combobox->addItem(QObject::tr("Recently Closed"));
  recently_closed_combobox->addItem("BNS.TSX");
  recently_closed_combobox->addItem("BMO.TSX");
  recently_closed_combobox->addItem("DIS.NYSE");
  recently_closed_combobox->addItem("WMT.NYSE");
  combobox_layout->addWidget(recently_closed_combobox);
  auto button_layout = new QHBoxLayout();
  button_layout->setContentsMargins(scale_width(8), scale_height(5), scale_width(8), scale_height(8));
  button_layout->setSpacing(0);
  input_layout->addLayout(button_layout);
  QPushButton* buttons[9];
  for(auto i = 0; i < 9; i++) {
    buttons[i] = new QPushButton("o", toolbar);
    buttons[i]->setFlat(true);
    buttons[i]->setStyleSheet("color: #513321;");
    buttons[i]->setFixedSize(scale(20, 20));
    button_layout->addWidget(buttons[i]);
  }


  toolbar->show();
  application->exec();
}