#include "Spire/LoginDialog.hpp"
#include <QMovie>
#include <QScreen>
#include "ui_LoginDialog.h"

using namespace Spire;

LoginDialog::LoginDialog(QWidget* parent) 
  : QDialog{parent},
    m_ui{std::make_unique<Ui_LoginDialog>()} {
  auto screen = qApp->screens().at(0);
  m_ui->setupUi(this);
  setStyleSheet("background-image: url(:/newPrefix/login-bg.png);");
  auto physicalDotsPerInchY = screen->physicalDotsPerInchY();
  auto physicalDotsPerInchX = screen->physicalDotsPerInchX();
  QFont f{"Roboto", 10, QFont::Normal};
  auto loginDialogHeightInches = 3.6;
  auto loginDialogWidthInches = 4.0;
  setFixedHeight(3.6 * physicalDotsPerInchY);
  setFixedWidth(4.0 * physicalDotsPerInchX);
  m_ui->m_leftSpacer->changeSize(0.425 * physicalDotsPerInchX, 0,
    QSizePolicy::Fixed);
  m_ui->m_rightSpacer->changeSize(0.5 * physicalDotsPerInchX, 0,
    QSizePolicy::Fixed);
  m_ui->m_bottomSpacer->changeSize(0, 0.0 * physicalDotsPerInchY,
    QSizePolicy::Fixed);
  m_ui->m_widget->move(0, 0);
  m_ui->m_widget->setFixedHeight(height());
  m_ui->m_widget->setFixedWidth(width());
  m_ui->m_widget->setStyleSheet("background:transparent;");
  m_ui->m_usernameInput->setFixedHeight(0.28 * physicalDotsPerInchY);
  m_ui->m_usernameInput->setFixedWidth(3.0 * physicalDotsPerInchX);
  m_ui->m_passwordInput->setFixedHeight(0.28 * physicalDotsPerInchY);
  m_ui->m_passwordInput->setFixedWidth(3.0 * physicalDotsPerInchX);
  m_ui->m_versionLabel->setFixedHeight(0.28 * physicalDotsPerInchY);
  m_ui->m_versionLabel->setFixedWidth(physicalDotsPerInchX);
  m_ui->m_versionLabel->setStyleSheet("color:rgb(255,255,255)");
  m_ui->m_versionLabel->setText("Version 1.0.0");
  m_ui->m_versionLabel->setFont(f);
  m_ui->m_versionLoginSpacer->changeSize(0.40 * physicalDotsPerInchX, 0,
    QSizePolicy::Fixed);
  auto movie = new QMovie(":/newPrefix/spire_desktop_loading_icon.gif");
  m_ui->m_loadingLabel->setMovie(movie);
  m_ui->m_loadingLabel->setFixedHeight(0.28 * physicalDotsPerInchY);
  m_ui->m_loadingLabel->setFixedWidth(physicalDotsPerInchX);
  m_ui->m_loadingLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  movie->start();
  m_ui->m_loginButton->setFixedHeight(0.28 * physicalDotsPerInchY);
  m_ui->m_loginButton->setFixedWidth(physicalDotsPerInchX);
  m_ui->m_loginButton->setFont(f);
  m_ui->m_loginButton->setStyleSheet(
    "background-color:rgb(104, 75, 199);"
    "color: white; border: 2px solid rgb(141,120,236);");
  m_ui->m_spireLogoLabel->setFixedHeight(0.5 * physicalDotsPerInchY);
  m_ui->m_spireLogoLabel->setFixedWidth(1.44 * physicalDotsPerInchX);
  m_ui->m_spireLogoLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  m_ui->m_spireLogoLabel->setStyleSheet(
    "background - image: url(: / newPrefix / spire - white.png);");
  m_ui->m_mainLayout->setAlignment(Qt::AlignCenter);
  m_ui->m_logoTopSpacer->changeSize(0, 0.4 * physicalDotsPerInchY,
    QSizePolicy::Fixed);
  m_ui->m_logoSpacer->changeSize(0, 0.25 * physicalDotsPerInchY,
    QSizePolicy::Fixed);
  m_ui->m_usernamePasswordSpacer->changeSize(0, 0.125 * physicalDotsPerInchY,
    QSizePolicy::Fixed);
  m_ui->m_passwordLoginSpacer->changeSize(0, 0.25 * physicalDotsPerInchY,
    QSizePolicy::Fixed);
  m_ui->m_invalidLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  m_ui->m_invalidLabel->setFont(f);
  m_ui->m_invalidLabel->setStyleSheet("color : rgb(219, 213, 44)");
}

LoginDialog::~LoginDialog() {}
