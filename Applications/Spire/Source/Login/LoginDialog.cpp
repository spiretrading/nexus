#include "Spire/Login/LoginDialog.hpp"
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <Beam/Threading/Threading.hpp>
#include <boost/lexical_cast.hpp>
#include <QApplication>
#include <QCoreApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QMovie>
#include <QPalette>
#include <QPixmap>
#include <QScreen>
#include <QtGui>
#include "ui_LoginDialog.h"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace boost;
using namespace Spire;
using namespace std;

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog{parent},
      m_ui{std::make_unique<Ui_LoginDialog>()} {
  setWindowFlags(Qt::FramelessWindowHint);
  m_ui->setupUi(this);
  auto screen = qApp->screens().at(0);
  auto physicalDotsPerInchY = screen->physicalDotsPerInchY();
  auto physicalDotsPerInchX = screen->physicalDotsPerInchX();
  QFont f{"Roboto", 10, QFont::Normal};
  const auto LOGIN_DIALOG_HEIGHT_INCHES = 3.6;
  const auto LOGIN_DIALOG_WIDTH_INCHES = 4.0;
  QPixmap pixmap{":/newPrefix/login-bg.png"};
  QBrush brush{pixmap.scaled(physicalDotsPerInchX * LOGIN_DIALOG_WIDTH_INCHES,
    LOGIN_DIALOG_HEIGHT_INCHES * physicalDotsPerInchY)};
  QPalette palette;
  palette.setBrush(QPalette::Background, brush);
  setPalette(palette);
  setFixedHeight(3.6 * physicalDotsPerInchY);
  setFixedWidth(4.0 * physicalDotsPerInchX);
  const auto LEFT_SPACER_SIZE = 0.425;
  m_ui->m_leftSpacer->changeSize(LEFT_SPACER_SIZE * physicalDotsPerInchX, 0,
    QSizePolicy::Fixed);
  const auto RIGHT_SPACER_SIZE = 0.5;
  m_ui->m_rightSpacer->changeSize(RIGHT_SPACER_SIZE * physicalDotsPerInchX, 0,
    QSizePolicy::Fixed);
  m_ui->m_bottomSpacer->changeSize(0, 0.5 * physicalDotsPerInchY,
    QSizePolicy::Fixed);
  m_ui->m_widget->move(0, 0);
  m_ui->m_widget->setFixedHeight(
    LOGIN_DIALOG_HEIGHT_INCHES * physicalDotsPerInchY);
  m_ui->m_widget->setFixedWidth(
    LOGIN_DIALOG_WIDTH_INCHES * physicalDotsPerInchX);
  m_ui->m_widget->setStyleSheet("background:transparent;");
  const auto USERNAME_INPUT_HEIGHT = 0.28;
  m_ui->m_usernameInput->setFixedHeight(USERNAME_INPUT_HEIGHT *
    physicalDotsPerInchY);
  m_ui->m_usernameInput->setFixedWidth(3.0 * physicalDotsPerInchX);
  const auto PASSWORD_INPUT_HEIGHT = 0.28;
  m_ui->m_usernameInput->setStyleSheet({ "background-color:rgb(255, 255, 255);"
    "color: rgb(168, 168, 168);"
    "color: rgb(0, 0, 0);"
    "font: 10pt 'Roboto';" });
  const auto LINE_EDIT_MARGIN = 0.1;
  m_ui->m_usernameInput->setTextMargins(LINE_EDIT_MARGIN * 
    physicalDotsPerInchX,0, 0, 0);
  m_ui->m_passwordInput->setFixedHeight(PASSWORD_INPUT_HEIGHT *
    physicalDotsPerInchY);
  m_ui->m_passwordInput->setFixedWidth(3.0 * physicalDotsPerInchX);
  m_ui->m_passwordInput->setStyleSheet({"background-color: rgb(255, 255, 255);"
    "color: rgb(168, 168, 168);"
    "color: rgb(0, 0, 0);"
    "font: 10pt 'Roboto';"});
  m_ui->m_passwordInput->setTextMargins(LINE_EDIT_MARGIN * 
    physicalDotsPerInchX, 0, 0, 0);
  const auto VERSION_LABEL_HEIGHT = 0.28;
  m_ui->m_versionLabel->setFixedHeight(VERSION_LABEL_HEIGHT *
    physicalDotsPerInchY);
  m_ui->m_versionLabel->setFixedWidth(1.2 * physicalDotsPerInchX);
  m_ui->m_versionLabel->setStyleSheet("color:rgb(255,255,255)");
  m_ui->m_versionLabel->setText("Version 1.0.0");
  m_ui->m_versionLabel->setFont(f);
  const auto VERSION_LOGIN_SPACER_WIDTH = 0.3;
  m_ui->m_versionLoginSpacer->changeSize(VERSION_LOGIN_SPACER_WIDTH *
    physicalDotsPerInchX, 0, QSizePolicy::Fixed);
  auto movie = new QMovie{":/newPrefix/spire_desktop_loading_icon.gif"};
  m_ui->m_loadingLabel->setMovie(movie);
  const auto LOADING_LABEL_HEIGHT = 0.28;
  m_ui->m_loadingLabel->setFixedHeight(LOADING_LABEL_HEIGHT *
    physicalDotsPerInchY);
  m_ui->m_loadingLabel->setFixedWidth(0.2288 * physicalDotsPerInchX);
  m_ui->m_loadingLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  movie->setScaledSize(m_ui->m_loadingLabel->size());
  m_ui->m_loadingLabel->setStyleSheet("background color : black");
  movie->start();
  const auto LOADING_LABEL_LOGIN_SPACER_WIDTH = 0.2;
  m_ui->m_loadingLabelLoginSpacer->changeSize(LOADING_LABEL_LOGIN_SPACER_WIDTH *
    physicalDotsPerInchX, 0, QSizePolicy::Fixed);
  const auto LOGIN_BUTTON_HEIGHT = 0.28;
  m_ui->m_loginButton->setFixedHeight(LOGIN_BUTTON_HEIGHT *
    physicalDotsPerInchY);
  m_ui->m_loginButton->setFixedWidth(physicalDotsPerInchX);
  m_ui->m_loginButton->setFont(f);
  m_ui->m_loginButton->setStyleSheet(
    "QPushButton{"
    "  background-color:rgb(104, 75, 199);"
    "  color: white;"
    "  border: 2px solid rgb(141,120,236);"
    "}"
    "QPushButton:hover{"
    "  background-color:rgb(141, 120, 236);"
    "  color: white;"
    "  border: 2px solid rgb(185,180,236);"
    "}"
    "QPushButton:pressed{"
    "  background-color:rgb(90, 58, 172);"
    "  color: white;"
    "  border: 2px solid rgb(104,75,199);"
    "}");
  const auto SPIRE_LOGO_LABEL_HEIGHT = 0.5;
  m_ui->m_spireLogoLabel->setFixedHeight(SPIRE_LOGO_LABEL_HEIGHT *
    physicalDotsPerInchY);
  const auto SPIRE_LOGO_LABEL_WIDTH = 1.44;
  m_ui->m_spireLogoLabel->setFixedWidth(SPIRE_LOGO_LABEL_WIDTH *
    physicalDotsPerInchX);
  m_ui->m_spireLogoLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  m_ui->m_spireLogoLabel->setStyleSheet(
    "border-image - image: url(: / newPrefix / spire - white.png);");
  m_ui->m_spireLogoInvalidSpacer->changeSize(0, 0.15 * physicalDotsPerInchY,
    QSizePolicy::Fixed);
  m_ui->m_mainLayout->setAlignment(Qt::AlignCenter);
  const auto LOGO_TOP_SPACER_HEIGHT = 0.8;
  m_ui->m_logoTopSpacer->changeSize(0, LOGO_TOP_SPACER_HEIGHT *
    physicalDotsPerInchY, QSizePolicy::Fixed);
  const auto LOGO_SPACER_HEIGHT = 0.25;
  m_ui->m_logoSpacer->changeSize(0, LOGO_SPACER_HEIGHT * physicalDotsPerInchY,
    QSizePolicy::Fixed);
  const auto USERNAME_PASSWORD_SPACER_HEIGHT = 0.125;
  m_ui->m_usernamePasswordSpacer->changeSize(0,
    USERNAME_PASSWORD_SPACER_HEIGHT * physicalDotsPerInchY, QSizePolicy::Fixed);
  const auto PASSWORD_LOGIN_SPACER = 0.25;
  m_ui->m_passwordLoginSpacer->changeSize(0, PASSWORD_LOGIN_SPACER *
    physicalDotsPerInchY, QSizePolicy::Fixed);
  m_ui->m_invalidLabel->setAlignment(Qt::AlignHCenter);
  m_ui->m_invalidLabel->setFont(f);
  m_ui->m_invalidLabel->setStyleSheet(
    "background-color : rgba(0,0,0,0%);"
    "color:rgb(219, 213, 44)");
}

LoginDialog::~LoginDialog() {}

void LoginDialog::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    m_mousePoint = event->pos();
  }
}

void LoginDialog::mouseMoveEvent(QMouseEvent* event) {
  if(event->buttons() & Qt::LeftButton) {
    auto diff = event->pos() - m_mousePoint;
    auto newpos = pos() + diff;
    move(newpos);
  }
}
