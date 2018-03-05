#include "spire/login/login_window.hpp"
#include <QHBoxLayout>
#include <QtSvg/QSvgRenderer>
#include <QVBoxlayout>
#include "spire/spire/dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;

login_window::login_window(const std::string& version, QWidget* parent)
    : QWidget(parent) {
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  setFixedSize(scale(384, 346));
  setStyleSheet("background-color: #4B23A0");
  auto layout = new QVBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(0);
  auto title_bar_layout = new QHBoxLayout;
  title_bar_layout->setMargin(0);
  title_bar_layout->setSpacing(0);
  title_bar_layout->addStretch(1);
  m_exit_button = new flat_button("", this);
  m_exit_button->setFixedSize(scale(32, 26));
  m_exit_button->setStyleSheet("background-image: url(:/icons/close.red.png)");
  title_bar_layout->addWidget(m_exit_button);
  layout->addLayout(title_bar_layout);
  layout->addStretch(30);
  auto logo_layout = new QHBoxLayout();
  logo_layout->setMargin(0);
  logo_layout->setSpacing(0);
  logo_layout->addStretch(1);
  m_logo_widget = new QSvgWidget(":/icons/logo.svg", this);
  m_logo_widget->setFixedSize(scale(134, 50));
  logo_layout->addWidget(m_logo_widget);
  logo_layout->addStretch(1);
  layout->addLayout(logo_layout);
  layout->addStretch(23);
  m_status_label = new QLabel("Incorrect text entered in this label.", this);
  m_status_label->setStyleSheet(QString(
    R"(color: #FAEB96;
       font-family: Roboto;
       font-size: %1px;
       qproperty-alignment: AlignCenter;)").arg(scale_height(12)));
  layout->addWidget(m_status_label);
  layout->addStretch(20);
  m_username_lineedit = new QLineEdit(this);
  m_username_lineedit->setPlaceholderText(tr("Username"));
  m_username_lineedit->setFixedHeight(scale_height(30));
  m_username_lineedit->setStyleSheet(QString(
    R"(background-color: white;
       border: 0px;
       margin: 0px %1px 0px %1px;
       padding-left: %2px;)").arg(scale_width(52)).arg(scale_width(10)));
  layout->addWidget(m_username_lineedit);
  layout->addStretch(15);
  m_password_lineedit = new QLineEdit(this);
  m_password_lineedit->setEchoMode(QLineEdit::Password);
  m_password_lineedit->setPlaceholderText("Password");
  m_password_lineedit->setFixedHeight(scale_height(30));
  m_password_lineedit->setStyleSheet(QString(
    R"(background-color: white;
       border: 0px;
       margin: 0px %1px 0px %1px;
       padding-left: %2px;)").arg(scale_width(52)).arg(scale_width(10)));
  layout->addWidget(m_password_lineedit);
  layout->addStretch(30);
  auto button_layout = new QHBoxLayout();
  button_layout->setMargin(0);
  button_layout->setSpacing(0);
  button_layout->addStretch(52);
  auto build_label = new QLabel(QString("Build ") + version.c_str(), this);
  build_label->setStyleSheet(QString(
    R"(color: white;
       font-family: Roboto;
       font-size: %1px;)").arg(scale_height(12)));
  build_label->setFixedSize(scale(160, 30));
  button_layout->addWidget(build_label);
  m_submit_button = new flat_button("Submit", this);
  m_submit_button->setFixedSize(scale(120, 30));
  button_layout->addWidget(m_submit_button);
  button_layout->addStretch(52);
  layout->addLayout(button_layout);
  layout->addStretch(48);
}

void login_window::set_state(state state) {
}

connection login_window::connect_login_signal(
    const login_signal::slot_type& slot) const {
  return m_login_signal.connect(slot);
}

connection login_window::connect_cancel_signal(
    const cancel_signal::slot_type& slot) const {
  return m_cancel_signal.connect(slot);
}
