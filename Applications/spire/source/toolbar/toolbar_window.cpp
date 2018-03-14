#include "spire/toolbar/toolbar_window.hpp"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QString>
#include <QVBoxlayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/icon_button.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;

toolbar_window::toolbar_window(QWidget* parent)
    : QWidget(parent) {
  setWindowFlag(Qt::FramelessWindowHint);
  setContentsMargins(0, 0, 0, 0);
  setFixedSize(scale(308, 98));
  setStyleSheet("background-color: #F5F5F5;");
  auto layout = new QVBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(0);

  auto title_bar_layout = new QHBoxLayout();
  title_bar_layout->setMargin(0);
  title_bar_layout->setSpacing(0);
  layout->addLayout(title_bar_layout);
  auto title_bar_logo = new QLabel("@", this);
  title_bar_logo->setStyleSheet("qproperty-alignment: AlignCenter;");
  title_bar_logo->setFixedSize(scale(26, 26));
  title_bar_layout->addWidget(title_bar_logo);
  auto username_label = new QLabel("Spire - Signed in, Username");
  username_label->setFixedSize(scale(218, 26));
  title_bar_layout->addWidget(username_label);
  auto minimize_button = new QLabel("-", this);
  minimize_button->setStyleSheet("qproperty-alignment: AlignCenter;");
  minimize_button->setFixedSize(scale(32, 26));
  title_bar_layout->addWidget(minimize_button);
  auto close_button = new QLabel("x", this);
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
  auto window_manager_combobox = new QComboBox(this);
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
  auto recently_closed_combobox = new QComboBox(this);
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
  auto test = new icon_button(":/icons/order-imbalances-purple.svg",
    ":/icons/order-imbalances-light-purple.svg", 20, 20, this);
  button_layout->addWidget(test);
}

connection toolbar_window::connect_closed_signal(
    const closed_signal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void toolbar_window::closeEvent(QCloseEvent* event) {
  m_closed_signal();
}
