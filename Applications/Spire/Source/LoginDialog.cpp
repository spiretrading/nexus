#include "Spire/LoginDialog.hpp"
#include <QMovie>
#include <QMouseEvent>
#include <QScreen>
#include <QtGui>
#include "ui_LoginDialog.h"


using namespace Spire;

LoginDialog::LoginDialog(QWidget* parent)
  : QDialog{ parent },
    m_ui{std::make_unique<Ui_LoginDialog>()} {
  setWindowFlags(Qt::FramelessWindowHint);
  m_ui->setupUi(this);
  setStyleSheet("background-image: url(:/newPrefix/login-bg.png);");
  auto screen = qApp->screens().at(0);
  auto physicalDotsPerInchY = screen->physicalDotsPerInchY();
  auto physicalDotsPerInchX = screen->physicalDotsPerInchX();
  QFont f{"Roboto", 10, QFont::Normal};
  const auto LOGIN_DIALOG_HEIGHT_INCHES = 3.6;
  const auto LOGIN_DIALOG_WIDTH_INCHES = 4.0;
  setFixedHeight(3.6 * physicalDotsPerInchY);
  setFixedWidth(4.0 * physicalDotsPerInchX);
  const auto M_LEFTSPACER_SIZE = 0.425;
  m_ui->m_leftSpacer->changeSize(M_LEFTSPACER_SIZE * physicalDotsPerInchX, 0,
    QSizePolicy::Fixed);
  const auto M_RIGHTSPACER_SIZE = 0.5;
  m_ui->m_rightSpacer->changeSize(M_RIGHTSPACER_SIZE * physicalDotsPerInchX, 0,
    QSizePolicy::Fixed);
  m_ui->m_bottomSpacer->changeSize(0, 0.0 * physicalDotsPerInchY,
    QSizePolicy::Fixed);
  m_ui->m_widget->move(0, 0);
  m_ui->m_widget->setFixedHeight(LOGIN_DIALOG_HEIGHT_INCHES*physicalDotsPerInchY);
  m_ui->m_widget->setFixedWidth(LOGIN_DIALOG_WIDTH_INCHES*physicalDotsPerInchX);
  m_ui->m_widget->setStyleSheet("background:transparent;");
  const auto M_USERNAMEINPUT_HEIGHT = 0.28;
  m_ui->m_usernameInput->setFixedHeight(M_USERNAMEINPUT_HEIGHT * 
    physicalDotsPerInchY);
  m_ui->m_usernameInput->setFixedWidth(3.0 * physicalDotsPerInchX);
  const auto M_PASSWORDINPUT_HEIGHT = 0.28;
  m_ui->m_passwordInput->setFixedHeight(M_PASSWORDINPUT_HEIGHT * 
    physicalDotsPerInchY);
  m_ui->m_passwordInput->setFixedWidth(3.0 * physicalDotsPerInchX);
  const auto M_VERSIONLABEL_HEIGHT = 0.28;
  m_ui->m_versionLabel->setFixedHeight(M_VERSIONLABEL_HEIGHT * 
    physicalDotsPerInchY);
  m_ui->m_versionLabel->setFixedWidth(1.15 * physicalDotsPerInchX);
  m_ui->m_versionLabel->setStyleSheet("color:rgb(255,255,255)");
  m_ui->m_versionLabel->setText("Version 1.0.0");
  m_ui->m_versionLabel->setFont(f);
  const auto M_VERSIONLOGINSPACER_WIDTH = 0.15; 
  m_ui->m_versionLoginSpacer->changeSize(M_VERSIONLOGINSPACER_WIDTH * 
    physicalDotsPerInchX, 0, QSizePolicy::Fixed);
  auto movie = new QMovie(":/newPrefix/spire_desktop_loading_icon.gif");
  m_ui->m_loadingLabel->setMovie(movie);
  const auto M_LOADINGLABEL_HEIGHT = 0.28;
  m_ui->m_loadingLabel->setFixedHeight(M_LOADINGLABEL_HEIGHT * 
    physicalDotsPerInchY);
  m_ui->m_loadingLabel->setFixedWidth(0.25 * physicalDotsPerInchX);
  m_ui->m_loadingLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  movie->start();
  const auto M_LOADINGLABELLOGINSPACER_WIDTH = 0.25;
  m_ui->m_loadingLabelLoginSpacer->changeSize(M_LOADINGLABELLOGINSPACER_WIDTH *
    physicalDotsPerInchX, 0, QSizePolicy::Fixed);
  const auto M_LOGINBUTTON_HEIGHT = 0.28;
  m_ui->m_loginButton->setFixedHeight(M_LOGINBUTTON_HEIGHT 
    * physicalDotsPerInchY);
  m_ui->m_loginButton->setFixedWidth(physicalDotsPerInchX);
  m_ui->m_loginButton->setFont(f);
  m_ui->m_loginButton->setStyleSheet(
    "background-color:rgb(104, 75, 199);"
    "color: white; border: 2px solid rgb(141,120,236);");
  const auto M_SPIRELOGOLABEL_HEIGHT = 0.5;
  m_ui->m_spireLogoLabel->setFixedHeight(M_SPIRELOGOLABEL_HEIGHT * 
    physicalDotsPerInchY);
  const auto M_SPIRELOGOLABEL_WIDTH = 1.44;
  m_ui->m_spireLogoLabel->setFixedWidth(M_SPIRELOGOLABEL_WIDTH * 
    physicalDotsPerInchX);
  m_ui->m_spireLogoLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  m_ui->m_spireLogoLabel->setStyleSheet(
    "background - image: url(: / newPrefix / spire - white.png);");
  m_ui->m_mainLayout->setAlignment(Qt::AlignCenter);
  const auto M_LOGOTOPSPACER_HEIGHT = 0.4;
  m_ui->m_logoTopSpacer->changeSize(0, M_LOGOTOPSPACER_HEIGHT *
    physicalDotsPerInchY, QSizePolicy::Fixed);
  const auto M_LOGOSPACER_HEIGHT = 0.25;
  m_ui->m_logoSpacer->changeSize(0, M_LOGOSPACER_HEIGHT * physicalDotsPerInchY,
    QSizePolicy::Fixed);
  const auto M_USERNAMEPASSWORDSPACER_HEIGHT = 0.125;
  m_ui->m_usernamePasswordSpacer->changeSize(0, 
    M_USERNAMEPASSWORDSPACER_HEIGHT * physicalDotsPerInchY,
    QSizePolicy::Fixed);
  const auto M_PASSWORDLOGINSPACER = 0.25;
  m_ui->m_passwordLoginSpacer->changeSize(0, M_PASSWORDLOGINSPACER *
    physicalDotsPerInchY, QSizePolicy::Fixed);
  m_ui->m_invalidLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  m_ui->m_invalidLabel->setFont(f);
  m_ui->m_invalidLabel->setStyleSheet("color : rgb(219, 213, 44)");
}

LoginDialog::~LoginDialog() {}

void LoginDialog::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    mousePoint = event->pos();
  }
}

void LoginDialog::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() & Qt::LeftButton) {
    auto diff = event->pos() - mousePoint;
    auto newpos = pos() + diff;
    move(newpos);
  }
}

